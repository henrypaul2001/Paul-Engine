#include "pepch.h"
#include "TextureImporter.h"

#include <stb_image.h>
#include <stb_image_write.h>
#include "PaulEngine/Project/Project.h"

#include "AssetManager.h"

#include "PaulEngine/Utils/YamlConversions.h"
#include "PaulEngine/Asset/BinarySerializer.h"

#include <yaml-cpp/yaml.h>

namespace PaulEngine
{
	void* STBI_LoadAs8Bit(const char* path, int* width, int* height, int* channels_in_file, int desired_channels, TextureImporter::ImageFileReadResult& out_result)
	{
		void* imageData = (void*)stbi_load(path, width, height, channels_in_file, desired_channels);
		out_result.BytesPerPixel = sizeof(uint8_t) * *channels_in_file;
		return imageData;
	}
	void* STBI_LoadAs16Bit(const char* path, int* width, int* height, int* channels_in_file, int desired_channels, TextureImporter::ImageFileReadResult& out_result)
	{
		void* imageData = (void*)stbi_load_16(path, width, height, channels_in_file, desired_channels);
		out_result.BytesPerPixel = sizeof(uint16_t) * *channels_in_file;
		return imageData;
	}
	void* STBI_LoadAs32Bit(const char* path, int* width, int* height, int* channels_in_file, int desired_channels, TextureImporter::ImageFileReadResult& out_result)
	{
		void* imageData = (void*)stbi_loadf(path, width, height, channels_in_file, desired_channels);
		out_result.BytesPerPixel = sizeof(uint32_t) * *channels_in_file;
		return imageData;
	}

	Buffer TextureImporter::ReadImageFile(const Load2DParams& loadParams, ImageFileReadResult& out_result)
	{
		PE_PROFILE_FUNCTION();
		std::string pathString = loadParams.Filepath.string();
		
		int width = 0, height = 0, channels = 0;
		stbi_set_flip_vertically_on_load(loadParams.FlipVertical);

		Load2DBitDepth loadAs = loadParams.DesiredBitDepth;
		if (loadAs == Load2DBitDepth::AUTO)
		{
			if (stbi_is_hdr(pathString.c_str())) { loadAs = Load2DBitDepth::BIT32; }
			else if (stbi_is_16_bit(pathString.c_str())) { loadAs = Load2DBitDepth::BIT16; }
			else { loadAs = Load2DBitDepth::BIT8; }
		}

		size_t bufferSize = 0;
		void* imageData = nullptr;
		switch (loadAs)
		{
		case BIT8:
			imageData = STBI_LoadAs8Bit(pathString.c_str(), &width, &height, &channels, loadParams.DesiredChannels, out_result);
			break;
		case BIT16:
			imageData = STBI_LoadAs16Bit(pathString.c_str(), &width, &height, &channels, loadParams.DesiredChannels, out_result);
			break;
		case BIT32:
			imageData = STBI_LoadAs32Bit(pathString.c_str(), &width, &height, &channels, loadParams.DesiredChannels, out_result);
			break;
		}

		bufferSize = out_result.BytesPerPixel * (width * height);

		if (imageData == nullptr)
		{
			PE_CORE_ERROR("Failed to load image at path: '{0}'", pathString);
			out_result.Width = 0;
			out_result.Height = 0;
			out_result.Channels = 0;
			out_result.BytesPerPixel = 0;
			return Buffer();
		}

		out_result.Width = width;
		out_result.Height = height;
		out_result.Channels = channels;
		return Buffer(imageData, bufferSize);
	}

	bool TextureImporter::SaveImageFile(std::filesystem::path filepath, const Buffer pixelData, const TextureSpecification spec, bool flipVertical, const uint8_t jpgQualityLevel)
	{
		stbi_flip_vertically_on_write(flipVertical);

		if (!pixelData)
		{
			PE_CORE_ERROR("Null buffer");
			return false;
		}

		if (spec.Width * spec.Height * PixelSize(spec.Format) != pixelData.Size())
		{
			PE_CORE_ERROR("Buffer / TextureSpecification mismatch");
			return false;
		}

		if (filepath.has_parent_path())
		{
			std::filesystem::path directory = filepath.parent_path();
			if (!std::filesystem::exists(directory))
			{
				PE_CORE_DEBUG("Creating directory '{0}'", directory.string());
				std::filesystem::create_directories(directory);
			}
		}

		if (!filepath.has_extension())
		{
			// Default to png
			filepath += ".png";
		}

		bool success = false;
		std::filesystem::path extension = filepath.extension();
		
		std::string extensionString = extension.string();
		std::transform(extensionString.begin(), extensionString.end(), extensionString.begin(), [](unsigned char c) { return std::tolower(c); } );
		if (extensionString == ".png")
		{
			success = SaveImagePNG(filepath, pixelData, spec);
		}
		else if (extensionString == ".bmp")
		{
			success = SaveImageBMP(filepath, pixelData, spec);
		}
		else if (extensionString == ".tga")
		{
			success = SaveImageTGA(filepath, pixelData, spec);
		}
		else if (extensionString == ".jpg")
		{
			success = SaveImageJPG(filepath, pixelData, spec, jpgQualityLevel);
		}
		else if (extensionString == ".hdr")
		{
			success = SaveImageHDR(filepath, pixelData, spec);
		}
		else
		{
			PE_CORE_ERROR("Unsupported image file extension: '{0}'", extension.string());
		}

		if (!success)
		{
			PE_CORE_ERROR("Failed to save image to path: '{0}'", filepath.string());
		}

		return success;
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

	bool TextureImporter::SaveImagePNG(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec)
	{
		return stbi_write_png(filepath.string().c_str(), spec.Width, spec.Height, NumChannels(spec.Format), (const void*)pixelData.m_Data, spec.Width * NumChannels(spec.Format));
	}

	bool TextureImporter::SaveImageBMP(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec)
	{
		return stbi_write_bmp(filepath.string().c_str(), spec.Width, spec.Height, NumChannels(spec.Format), (const void*)pixelData.m_Data);
	}

	bool TextureImporter::SaveImageTGA(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec)
	{
		return stbi_write_tga(filepath.string().c_str(), spec.Width, spec.Height, NumChannels(spec.Format), (const void*)pixelData.m_Data);
	}

	bool TextureImporter::SaveImageJPG(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec, const uint8_t qualityLevel)
	{
		return stbi_write_jpg(filepath.string().c_str(), spec.Width, spec.Height, NumChannels(spec.Format), (const void*)pixelData.m_Data, (int)qualityLevel);
	}

	bool TextureImporter::SaveImageHDR(const std::filesystem::path& filepath, const Buffer pixelData, const TextureSpecification spec)
	{
		if (!Is32BitLinearFormat(spec.Format))
		{
			PE_CORE_ERROR("HDR image format must be 32 bit linear (non interleaved)");
			return false;
		}
		return stbi_write_hdr(filepath.string().c_str(), spec.Width, spec.Height, NumChannels(spec.Format), pixelData.As<float>());
	}

	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		// Disabled 16bit loading due to issues with problematic assets producing unexpected results 
		// (example: TestProject/assets/textures/brick_wall/displacement.png has two 32 bit channels 
		// red and alpha which stbi perceives as a 16 bit texture)
		Load2DParams params = Project::GetAssetDirectory() / metadata.FilePath;
		params.DesiredBitDepth = Load2DBitDepth::BIT8;
		Ref<Texture2D> texture = LoadTexture2D(params);
		texture->Handle = handle;
		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const Load2DParams& loadParams)
	{
		PE_PROFILE_FUNCTION();

		ImageFileReadResult result;
		Buffer data = ReadImageFile(loadParams, result);

		TextureSpecification spec;
		spec.Width = result.Width;
		spec.Height = result.Height;

		spec.Format = BuildImageFormat(result.Channels, result.BytesPerPixel);
		if (spec.Format == ImageFormat::None)
		{
			PE_CORE_WARN("No image format matches input: {0} channel(s) with {1} bytes per pixel", result.Channels, result.BytesPerPixel);
			
			// Fall back to num channels only
			spec.Format = BuildImageFormat(result.Channels, result.Channels);
			PE_CORE_ASSERT(spec.Format != ImageFormat::None, "Failed to create image format from file read");
		}

		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		data.Release();

		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const Load2DParams& loadParams, TextureSpecification spec)
	{
		PE_PROFILE_FUNCTION();
		
		ImageFileReadResult result;
		Buffer data = ReadImageFile(loadParams, result);

		uint8_t specChannels = (uint8_t)NumChannels(spec.Format);
		uint8_t specBPP = (uint8_t)PixelSize(spec.Format);
		if (result.Channels != specChannels || result.BytesPerPixel != specBPP)
		{
			PE_CORE_WARN("ImageFormat mismatch in LoadTexture2D. Overriding image format");
			PE_CORE_WARN("    - Channels found  in image: {0}", result.Channels);
			PE_CORE_WARN("    - Channels found  in  spec: {0}", specChannels);
			PE_CORE_WARN("    - Bytes per pixel in image: {0}", result.BytesPerPixel);
			PE_CORE_WARN("    - Bytes per pixel in  spec: {0}", specBPP);

			spec.Format = BuildImageFormat(result.Channels, result.BytesPerPixel);
			if (spec.Format == ImageFormat::None)
			{
				PE_CORE_ERROR("No image format matches input: {0} channel(s) with {1} bytes per pixel", result.Channels, result.BytesPerPixel);
				PE_CORE_ASSERT(false, "Failed to create image format from file read");
			}
		}

		spec.Width = result.Width;
		spec.Height = result.Height;

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