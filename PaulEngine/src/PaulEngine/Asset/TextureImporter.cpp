#include "pepch.h"
#include "TextureImporter.h"

#include <stb_image.h>
#include "PaulEngine/Project/Project.h"

#include "AssetManager.h"

#include "PaulEngine/Utils/YamlConversions.h"
#include "PaulEngine/Asset/BinarySerializer.h"

#include <yaml-cpp/yaml.h>

namespace PaulEngine
{
	Buffer TextureImporter::ReadImageFile(const std::filesystem::path& filepath, ImageFileReadResult& out_result, bool flipVertical)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(flipVertical);

		unsigned char* imageData = nullptr;
		{
			PE_PROFILE_SCOPE("stbi_load image");
			std::string pathString = filepath.string();
			imageData = stbi_load(pathString.c_str(), &width, &height, &channels, 0);
		}

		if (imageData == nullptr)
		{
			PE_CORE_ERROR("Failed to load image at path: '{0}'", filepath.string());
			out_result.Width = 0;
			out_result.Height = 0;
			out_result.Channels = 0;
			return Buffer();
		}

		out_result.Width = width;
		out_result.Height = height;
		out_result.Channels = channels;
		return Buffer(imageData, sizeof(unsigned char) * (width * height * channels));
	}

	Ref<Texture2DArray> TextureImporter::ImportTexture2DArray(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Texture2DArray> textureArray = ReadBTAFile(Project::GetAssetDirectory() / metadata.FilePath);
		textureArray->Handle = handle;
		return textureArray;
	}

	Ref<Texture2DArray> TextureImporter::ReadBTAFile(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".bta", "Invalid file extension");

		std::ifstream fin;
		fin.open(filepath, std::ios::in | std::ios::binary);

		PE_CORE_ASSERT(fin.is_open(), "Unable to open file");

		TextureSpecification spec = ReadTextureSpecBinary(fin);

		int numLayers = 0;
		uint64_t bufferSize = 0;

		// Read layer info
		fin.read((char*)&numLayers, sizeof(int));
		fin.read((char*)&bufferSize, sizeof(uint64_t));

		std::vector<Buffer> layerBuffers;

		// Read buffers
		for (int i = 0; i < numLayers; i++) {
			layerBuffers.push_back(BinarySerializer::ReadBuffer(fin, bufferSize));
		}

		fin.close();

		Ref<Texture2DArray> textureArray = Texture2DArray::Create(spec, layerBuffers);

		for (Buffer b : layerBuffers) {
			b.Release();
		}

		return textureArray;
	}

	bool TextureImporter::SaveBTAFile(const std::filesystem::path& filepath, std::vector<Buffer> layerBuffers, const TextureSpecification spec)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".bta", "Invalid file extension");

		int numLayers = layerBuffers.size();
		PE_CORE_ASSERT(numLayers > 1, "Must include more than one buffer");
		
		uint64_t bufferSize = layerBuffers[0].Size();
		for (int i = 1; i < layerBuffers.size(); i++)
		{
			PE_CORE_ASSERT(layerBuffers[i].Size() == bufferSize, "All buffers must be the same size for binary texture array file");
		}

		int width = spec.Width;
		int height = spec.Height;
		int channels = NumChannels(spec.Format);

		PE_CORE_ASSERT(bufferSize == width * height * channels, "Invalid texture spec for buffer size");

		std::error_code error;
		std::filesystem::create_directories(filepath.parent_path(), error);

		std::ofstream fout;
		fout.open(filepath, std::ios::out | std::ios::binary);

		WriteTextureSpecBinary(fout, spec);

		// Write layer info
		fout.write((char*)&numLayers, sizeof(int));
		fout.write((char*)&bufferSize, sizeof(uint64_t));

		// Write buffers
		for (Buffer& b : layerBuffers) {
			if (!BinarySerializer::WriteBuffer(fout, b)) { fout.close(); return false; }
		}

		fout.close();
		return true;
	}

	Ref<TextureCubemap> TextureImporter::ImportTextureCubemap(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<TextureCubemap> cubemap = LoadTextureCubemap(Project::GetAssetDirectory() / metadata.FilePath);
		cubemap->Handle = handle;
		return cubemap;
	}

	Ref<TextureCubemap> TextureImporter::LoadTextureCubemap(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".ccm", "Invalid file extension");

		std::ifstream fin;
		fin.open(filepath, std::ios::in | std::ios::binary);

		PE_CORE_ASSERT(fin.is_open(), "Unable to open file");

		TextureSpecification spec = ReadTextureSpecBinary(fin);

		// Read faces
		std::vector<Buffer> uncompressedFaces = std::vector<Buffer>(6);
		for (int i = 0; i < 6; i++)
		{
			size_t uncompressedSize = 0;
			size_t compressedSize = 0;
			fin.read((char*)&uncompressedSize, sizeof(size_t));
			fin.read((char*)&compressedSize, sizeof(size_t));

			Buffer compressedFace = BinarySerializer::ReadBuffer(fin, compressedSize);
			uncompressedFaces[i] = BinarySerializer::UncompressBuffer(compressedFace, uncompressedSize);
			compressedFace.Release();
		}

		fin.close();

		Ref<TextureCubemap> cubemap = TextureCubemap::Create(spec, uncompressedFaces);

		for (Buffer b : uncompressedFaces) {
			b.Release();
		}

		return cubemap;
	}

	bool TextureImporter::SaveCubemapFile(const std::filesystem::path& filepath, const Buffer uncompressedFaces[6], const TextureSpecification spec)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".ccm", "Invalid file extension");
		
		size_t uncompressedSize = uncompressedFaces[0].Size();
		for (int i = 1; i < 6; i++)
		{
			PE_CORE_ASSERT(uncompressedFaces[i].Size() == uncompressedSize, "Cubemap faces must be same uncompressed size");
		}
		
		int width = spec.Width;
		int height = spec.Height;
		int channels = NumChannels(spec.Format);
		
		PE_CORE_ASSERT(uncompressedSize == width * height * channels, "Invalid texture spec for buffer size");
		
		std::error_code error;
		std::filesystem::create_directories(filepath.parent_path(), error);
		
		std::ofstream fout;
		fout.open(filepath, std::ios::out | std::ios::binary);
		
		WriteTextureSpecBinary(fout, spec);

		// Write faces
		for (int i = 0; i < 6; i++)
		{
			Buffer compressedFace = BinarySerializer::CompressBuffer(uncompressedFaces[i]);
			size_t compressedSize = compressedFace.Size();
			fout.write((char*)&uncompressedSize, sizeof(size_t));
			fout.write((char*)&compressedSize, sizeof(size_t));

			if (!BinarySerializer::WriteBuffer(fout, compressedFace)) { fout.close(); compressedFace.Release(); return false; }
			compressedFace.Release();
		}
		
		fout.close();
		return true;
	}

	void TextureImporter::WriteTextureSpecBinary(std::ofstream& fout, const TextureSpecification& spec)
	{
		fout.write((char*)&spec.Format, sizeof(ImageFormat));
		fout.write((char*)&spec.MinFilter, sizeof(ImageMinFilter));
		fout.write((char*)&spec.MagFilter, sizeof(ImageMagFilter));
		fout.write((char*)&spec.Wrap_S, sizeof(ImageWrap));
		fout.write((char*)&spec.Wrap_T, sizeof(ImageWrap));
		fout.write((char*)&spec.Wrap_R, sizeof(ImageWrap));
		fout.write((char*)&spec.Border, sizeof(glm::vec4));
		fout.write((char*)&spec.Width, sizeof(uint32_t));
		fout.write((char*)&spec.Height, sizeof(uint32_t));
		fout.write((char*)&spec.GenerateMips, sizeof(bool));
	}

	TextureSpecification TextureImporter::ReadTextureSpecBinary(std::ifstream& fin)
	{
		TextureSpecification spec;
		fin.read((char*)&spec.Format, sizeof(ImageFormat));
		fin.read((char*)&spec.MinFilter, sizeof(ImageMinFilter));
		fin.read((char*)&spec.MagFilter, sizeof(ImageMagFilter));
		fin.read((char*)&spec.Wrap_S, sizeof(ImageWrap));
		fin.read((char*)&spec.Wrap_T, sizeof(ImageWrap));
		fin.read((char*)&spec.Wrap_R, sizeof(ImageWrap));
		fin.read((char*)&spec.Border, sizeof(glm::vec4));
		fin.read((char*)&spec.Width, sizeof(uint32_t));
		fin.read((char*)&spec.Height, sizeof(uint32_t));
		fin.read((char*)&spec.GenerateMips, sizeof(bool));
		return spec;
	}

	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Texture2D> texture = LoadTexture2D(Project::GetAssetDirectory() / metadata.FilePath);
		texture->Handle = handle;
		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();

		ImageFileReadResult result;
		Buffer data = ReadImageFile(filepath, result);

		TextureSpecification spec;
		spec.Width = result.Width;
		spec.Height = result.Height;

		switch (result.Channels)
		{
		case 4:
			spec.Format = ImageFormat::RGBA8;
			break;
		case 3:
			spec.Format = ImageFormat::RGB8;
			break;
		case 2:
			spec.Format = ImageFormat::RG8;
			break;
		case 1:
			spec.Format = ImageFormat::R8;
			break;
		}

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		data.Release();

		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& filepath, TextureSpecification spec)
	{
		PE_PROFILE_FUNCTION();

		ImageFileReadResult result;
		Buffer data = ReadImageFile(filepath, result);

		spec.Width = result.Width;
		spec.Height = result.Height;

		int channels = result.Channels;
		bool validateChannels = ValidateChannels(spec.Format, result.Channels);

		if (!validateChannels)
		{
			PE_CORE_WARN("ImageFormat mismatch in LoadTexture2D. Overriding image format");
			PE_CORE_WARN("    - Channels found in image: {0}", result.Channels);
			PE_CORE_WARN("    - Channels found in  spec: {0}", channels);

			switch (result.Channels)
			{
			case 4:
				spec.Format = ImageFormat::RGBA8;
				break;
			case 3:
				spec.Format = ImageFormat::RGB8;
				break;
			case 2:
				spec.Format = ImageFormat::RG8;
				break;
			case 1:
				spec.Format = ImageFormat::R8;
				break;
			}
		}

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		data.Release();

		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const Buffer dataBuffer, TextureSpecification spec)
	{
		PE_PROFILE_FUNCTION();
		return Texture2D::Create(spec, dataBuffer);
	}

	Ref<TextureAtlas2D> TextureImporter::ImportTextureAtlas2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<TextureAtlas2D> textureAtlas = LoadTextureAtlas2D(Project::GetAssetDirectory() / metadata.FilePath);
		textureAtlas->Handle = handle;
		return textureAtlas;
	}

	Ref<TextureAtlas2D> TextureImporter::LoadTextureAtlas2D(const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".patlas", "Invalid file extension");

		std::ifstream stream = std::ifstream(filepath);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		if (!data["TextureAtlas2D"]) { return nullptr; }

		std::string atlasName = data["TextureAtlas2D"].as<std::string>();
		PE_CORE_TRACE("Deserializing texture atlas '{0}'", atlasName);

		AssetHandle baseTextureHandle = data["TextureHandle"].as<AssetHandle>();
		TextureAtlas2D textureAtlas = TextureAtlas2D(baseTextureHandle);

		YAML::Node subtextures = data["SubTextures"];
		if (subtextures) {
			for (YAML::Node subtexture : subtextures) {
				SubTexture2D subTexture2D = SubTexture2D(baseTextureHandle, { 0.0f, 0.0f }, { 0.0f, 0.0f });
				std::string name = subtexture["Name"].as<std::string>();

				YAML::Node texCoords = subtexture["TexCoords"];
				if (texCoords) {
					int count = 0;
					for (YAML::Node coord : texCoords) {
						subTexture2D.m_TexCoords[count] = coord.as<glm::vec2>();
						count++;
					}
				}

				textureAtlas.AddSubTexture(name, subTexture2D);
			}
		}
		
		return CreateRef<TextureAtlas2D>(textureAtlas);
	}

	void TextureImporter::SaveTextureAtlas2D(const TextureAtlas2D textureAtlas, const std::filesystem::path& filepath)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(filepath.extension() == ".patlas", "Invalid file extension");

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "TextureAtlas2D" << YAML::Value << filepath.stem().string();
		out << YAML::Key << "TextureHandle" << YAML::Value << textureAtlas.m_BaseTextureHandle;
		out << YAML::Key << "SubTextures" << YAML::Value << YAML::BeginSeq;

		for (auto& it : textureAtlas.m_SubTextureMap) {
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << it.first;

			out << YAML::Key << "TexCoords" << YAML::Value << YAML::BeginSeq;
			for (int i = 0; i < 4; i++) {
				glm::vec2 coord = it.second.m_TexCoords[i];
				out << YAML::Flow;
				out << YAML::BeginSeq << coord.x << coord.y << YAML::EndSeq;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::error_code error;
		std::filesystem::create_directories(filepath.parent_path(), error);
		std::ofstream fout = std::ofstream(filepath);
		fout << out.c_str();
	}

	Ref<EnvironmentMap> TextureImporter::ImportEnvironmentMap(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<EnvironmentMap> envMap = LoadEnvironmentMap(Project::GetAssetDirectory() / metadata.FilePath, metadata.Persistent);
		envMap->Handle = handle;
		return envMap;
	}

	Ref<EnvironmentMap> TextureImporter::LoadEnvironmentMap(const std::filesystem::path& filepath, bool persistentAsset)
	{
		return CreateRef<EnvironmentMap>(filepath, persistentAsset);
	}
}