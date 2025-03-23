#include "pepch.h"

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
}