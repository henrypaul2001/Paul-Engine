#include "pepch.h"
#include "TextureImporter.h"

#include <stb_image.h>
#include "PaulEngine/Project/Project.h"

#include <yaml-cpp/yaml.h>

// Type conversions
namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs) {
			if (!node.IsSequence() || node.size() != 2) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<PaulEngine::UUID>
	{
		static Node encode(const PaulEngine::UUID& uuid) {
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, PaulEngine::UUID& uuid) {
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

namespace PaulEngine
{
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

		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);

		unsigned char* imageData = nullptr;
		{
			PE_PROFILE_SCOPE("stbi_load image");
			std::string pathString = filepath.string();
			imageData = stbi_load(pathString.c_str(), &width, &height, &channels, 0);
		}

		if (imageData == nullptr)
		{
			PE_CORE_ERROR("Failed to load image at path: '{0}'", filepath.string());
			return nullptr;
		}

		Buffer data = Buffer(imageData, sizeof(unsigned char) * (width * height * channels));

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;

		switch (channels)
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

				textureAtlas.SetSubTexture(name, subTexture2D);
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
}