#include "pepch.h"

#include <yaml-cpp/yaml.h>

#include "PaulEngine/Project/Project.h"
#include "AssetImporter.h"
#include "EditorAssetManager.h"

namespace PaulEngine
{
	static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
		{ ".paul", AssetType::Scene },
		{ ".png", AssetType::Texture2D },
		{ ".jpg", AssetType::Texture2D },
		{ ".jpeg", AssetType::Texture2D },
		{ ".ttf", AssetType::Font },
		{ ".patlas", AssetType::TextureAtlas2D }
	};

	static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension) {
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end()) {
			PE_CORE_WARN("Could not find AssetType for file extension '{0}'", extension.string().c_str());
			return AssetType::None;
		}
		
		return s_AssetExtensionMap.at(extension);
	}

	bool EditorAssetManager::IsAssetRegistered(AssetHandle handle) const
	{
		return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return (IsAssetPersistentLoaded(handle) || IsAssetTempLoaded(handle));
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
	{
		if (!IsAssetRegistered(handle)) {
			return AssetType::None;
		}
		return m_AssetRegistry.at(handle).Type;
	}

	bool EditorAssetManager::IsAssetTempLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
	}

	bool EditorAssetManager::IsAssetPersistentLoaded(AssetHandle handle) const
	{
		return m_LoadedPersistentAssets.find(handle) != m_LoadedPersistentAssets.end();
	}

	void EditorAssetManager::ReleaseTempAssets()
	{
		m_LoadedAssets.clear();
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath, const bool persistent)
	{
		if (IsAssetRegistered(filepath)) {
			return m_AssetFileRegistry.at(filepath);
		}

		AssetHandle handle;
		AssetMetadata metadata;
		metadata.FilePath = filepath;
		metadata.Type = GetAssetTypeFromFileExtension(filepath.extension());
		if (metadata.Type == AssetType::None) {
			return AssetHandle(0);
		}
		metadata.Persistent = persistent;

		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		if (asset)
		{
			asset->Handle = handle;
			AddToLoadedAssets(asset, persistent);
			RegisterAsset(handle, metadata);
			SerializeAssetRegistry();
		}

		return handle;
	}

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		if (!IsAssetRegistered(handle)) {
			return nullptr;
		}

		Ref<Asset> asset;
		if (IsAssetTempLoaded(handle)) {
			asset = m_LoadedAssets.at(handle);
		}
		else if (IsAssetPersistentLoaded(handle)) {
			asset = m_LoadedPersistentAssets.at(handle);
		}
		else {
			// Load asset if not already loaded
			const AssetMetadata& metadata = GetMetadata(handle);
			asset = AssetImporter::ImportAsset(handle, metadata);
			if (!asset) {
				PE_CORE_ERROR("Asset import failed at path '{0}'", metadata.FilePath.string().c_str());
			}
			AddToLoadedAssets(asset, metadata.Persistent);
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

	const std::filesystem::path& EditorAssetManager::GetFilepath(AssetHandle handle) const
	{
		return GetMetadata(handle).FilePath;
	}

	void EditorAssetManager::RegisterAsset(AssetHandle handle, AssetMetadata metadata)
	{
		m_AssetRegistry[handle] = metadata;
		m_AssetFileRegistry[metadata.FilePath] = handle;
	}

	void EditorAssetManager::AddToLoadedAssets(Ref<Asset> asset, bool persistent)
	{
		if (persistent) {
			m_LoadedPersistentAssets[asset->Handle] = asset;
		}
		else {
			m_LoadedAssets[asset->Handle] = asset;
		}
	}

	void EditorAssetManager::UnloadAsset(AssetHandle& handle)
	{
		if (IsAssetTempLoaded(handle)) {
			if (m_LoadedAssets.erase(handle) == 0) {
				PE_CORE_WARN("Attempted to unload temp asset not currently loaded with handle '{0}'", (uint64_t)handle);
			}
		}
		else if (IsAssetPersistentLoaded(handle)) {
			if (m_LoadedPersistentAssets.erase(handle) == 0) {
				PE_CORE_WARN("Attempted to unload persistent asset not currently loaded with handle '{0}'", (uint64_t)handle);
			}
		}
		else {
			PE_CORE_WARN("Attempted to unload asset not currently loaded with handle '{0}'", (uint64_t)handle);
		}
		handle = 0;
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

				out << YAML::Key << "Persistent" << YAML::Value << metadata.Persistent;

				out << YAML::Value << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);
		std::ofstream fout = std::ofstream(path);
		fout << out.c_str();
	}

	bool EditorAssetManager::DeserializeAssetRegistry()
	{
		const std::filesystem::path& path = Project::GetAssetRegistryPath();

		std::ifstream stream = std::ifstream(path);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data;
		try
		{
			data = YAML::Load(ss.str());
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
			metadata.Persistent = node["Persistent"].as<bool>();
			metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
			m_AssetFileRegistry[metadata.FilePath] = handle;
		}

		return true;
	}
}