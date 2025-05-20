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
		{ ".patlas", AssetType::TextureAtlas2D },
		{ ".glsl", AssetType::Shader },
		{ ".pmat", AssetType::Material },
		{ ".bta", AssetType::Texture2DArray }
	};

	static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension) {
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end()) {
			PE_CORE_WARN("Could not find AssetType for file extension '{0}'", extension.string().c_str());
			return AssetType::None;
		}
		
		return s_AssetExtensionMap.at(extension);
	}

	Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		if (!IsAssetRegistered(handle)) {
			return nullptr;
		}

		Ref<Asset> asset;
		if (IsAssetTempLoaded(handle)) {
			asset = m_TempAssets.at(handle);
		}
		else if (IsAssetPersistentLoaded(handle)) {
			asset = m_PersistentAssets.at(handle);
		}
		else {
			const AssetMetadata& metadata = GetMetadata(handle);
			if (metadata.FilePath.empty()) {
				// Procedural asset
				// For now this will return an empty asset
				// Design consideration would be to have this path return 
				// a newly created asset with the requested type as a fallback 
				// when that original asset no longer exists (such as this case)
				PE_CORE_WARN("Procedural asset with handle '{0}' no longer exists", (uint64_t)handle);
				return asset;
			}

			// Load filepath asset from source
			asset = AssetImporter::ImportAsset(handle, metadata);
			if (asset)
			{
				asset->Handle = handle;
				AddToLoadedAssets(asset, metadata.Persistent);
				return asset;
			}

			PE_CORE_ERROR("Asset import failed at path '{0}'", metadata.FilePath.string().c_str());
		}
		return asset;
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
	{
		if (!IsAssetRegistered(handle)) {
			return AssetType::None;
		}
		return m_AssetRegistry.at(handle).Type;
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

	bool EditorAssetManager::IsAssetTempLoaded(AssetHandle handle) const
	{
		return (m_TempAssets.find(handle) != m_TempAssets.end());
	}

	bool EditorAssetManager::IsAssetPersistentLoaded(AssetHandle handle) const
	{
		return (m_PersistentAssets.find(handle) != m_PersistentAssets.end());
	}

	bool EditorAssetManager::IsAssetRegistered(AssetHandle handle) const
	{
		return (m_AssetRegistry.find(handle) != m_AssetRegistry.end());
	}

	bool EditorAssetManager::IsSourceFileRegistered(const std::filesystem::path& filepath) const
	{
		return (m_SourceFileRegistry.find(filepath) != m_SourceFileRegistry.end());
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return (IsAssetTempLoaded(handle) || IsAssetPersistentLoaded(handle));
	}

	bool EditorAssetManager::IsAssetProcedural(AssetHandle handle) const
	{
		const AssetMetadata& metadata = GetMetadata(handle);
		if (metadata.Type != AssetType::None) {
			return (metadata.FilePath.empty());
		}
		return false;
	}

	void EditorAssetManager::UnloadAsset(AssetHandle& handle)
	{
		if (IsAssetTempLoaded(handle)) {
			if (m_TempAssets.erase(handle) == 0) {
				PE_CORE_WARN("Attempted to unload temp asset not currently loaded with handle '{0}'", (uint64_t)handle);
			}
		}
		else if (IsAssetPersistentLoaded(handle)) {
			if (m_PersistentAssets.erase(handle) == 0) {
				PE_CORE_WARN("Attempted to unload persistent asset not currently loaded with handle '{0}'", (uint64_t)handle);
			}
		}
		else {
			PE_CORE_WARN("Attempted to unload asset not currently loaded with handle '{0}'", (uint64_t)handle);
		}
		handle = 0;
	}

	void EditorAssetManager::ReleaseTempAssets()
	{
		m_TempAssets.clear();
	}

	void EditorAssetManager::RegisterAsset(AssetHandle handle, AssetMetadata metadata)
	{
		if (handle != 0) {
			m_AssetRegistry[handle] = metadata;
			if (!metadata.FilePath.empty()) {
				m_SourceFileRegistry[metadata.FilePath] = handle;
			}
		}
	}

	AssetHandle EditorAssetManager::ImportAssetFromFile(const std::filesystem::path& filepath, const bool persistent)
	{
		if (IsSourceFileRegistered(filepath)) {
			return m_SourceFileRegistry.at(filepath);
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

	void EditorAssetManager::SerializeAssetRegistry()
	{
		const std::filesystem::path& path = Project::GetAssetRegistryPath();

		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry) {
				if (!IsAssetProcedural(handle))
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Handle" << YAML::Value << handle;

					std::string filepathString = metadata.FilePath.string();
					out << YAML::Key << "Filepath" << YAML::Value << filepathString;

					out << YAML::Key << "Persistent" << YAML::Value << metadata.Persistent;

					out << YAML::Value << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
					out << YAML::EndMap;
				}
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
			m_SourceFileRegistry[metadata.FilePath] = handle;
		}

		return true;
	}

	void EditorAssetManager::AddToLoadedAssets(Ref<Asset> asset, bool persistent)
	{
		if (persistent) {
			m_PersistentAssets[asset->Handle] = asset;
		}
		else {
			m_TempAssets[asset->Handle] = asset;
		}
	}
}