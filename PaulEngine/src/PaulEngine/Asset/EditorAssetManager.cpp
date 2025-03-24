#include "pepch.h"

#include <yaml-cpp/yaml.h>

#include "PaulEngine/Project/Project.h"
#include "AssetImporter.h"
#include "EditorAssetManager.h"

namespace PaulEngine
{
	bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	void EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
	{
		AssetHandle handle;
		AssetMetadata metadata;
		metadata.FilePath = filepath;
		metadata.Type = AssetType::Texture2D; // TODO: hardcoded texture2D asset type for now
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		asset->Handle = handle;
		
		if (asset)
		{
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			SerializeAssetRegistry();
		}
	}

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		if (!IsAssetHandleValid(handle)) {
			return nullptr;
		}

		Ref<Asset> asset;
		if (IsAssetLoaded(handle)) {
			asset = m_LoadedAssets.at(handle);
		}
		else {
			// Load asset if not already loaded
			const AssetMetadata& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(handle, metadata);
			if (!asset) {
				PE_CORE_ERROR("Asset import failed at path '{0}'", metadata.FilePath.string().c_str());
			}
		}

		return asset;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
	{
		static AssetMetadata s_NullMetadata;
		auto it = m_AssetRegistry.find(handle);
		if (it == m_AssetRegistry.end()) {
			return s_NullMetadata;
		}
		return it->second;
	}

	void EditorAssetManager::SerializeAssetRegistry()
	{
		const std::filesystem::path& path = Project::GetAssetRegistryPath();

		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry) {
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;

				std::string filepathString = metadata.FilePath.string();
				out << YAML::Key << "Filepath" << YAML::Value << filepathString;

				out << YAML::Value << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		std::ofstream fout = std::ofstream(path);
		fout << out.c_str();
	}

	bool EditorAssetManager::DeserializeAssetRegistry()
	{
		const std::filesystem::path& path = Project::GetAssetRegistryPath();

		YAML::Node data;
		try
		{
			data = YAML::Load(path.string());
		}
		catch (YAML::ParserException e)
		{
			PE_CORE_ERROR("Failed to load asset registry at path: '{0}'\n    - {1}", path.string().c_str(), e.what());
			return false;
		}

		YAML::Node rootNode = data["AssetRegistry"];
		if (!rootNode) {
			PE_CORE_ERROR("Failed to load asset registry at path: '{0}'\n    - Invalid format");
			return false;
		}

		for (const auto& node : rootNode) {
			AssetHandle handle = node["Handle"].as<uint64_t>();
			auto& metadata = m_AssetRegistry[handle];
			metadata.FilePath = node["Filepath"].as<std::string>();
			metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
		}

		return true;
	}
}