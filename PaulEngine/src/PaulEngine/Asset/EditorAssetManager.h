#pragma once
#include "AssetManagerBase.h"
#include "Asset.h"

#include <unordered_map>

namespace PaulEngine
{
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	using AssetFileRegistry = std::unordered_map<std::filesystem::path, AssetHandle>;
	class EditorAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;

		virtual bool IsAssetRegistered(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;

		virtual AssetType GetAssetType(AssetHandle handle) const override;

		bool IsAssetTempLoaded(AssetHandle handle) const;
		bool IsAssetPersistentLoaded(AssetHandle handle) const;

		void ReleaseTempAssets();

		AssetHandle ImportAsset(const std::filesystem::path& filepath, const bool persistent = false);
		bool IsAssetRegistered(const std::filesystem::path& filepath) {
			return (m_AssetFileRegistry.find(filepath) != m_AssetFileRegistry.end());
		}

		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		const std::filesystem::path& GetFilepath(AssetHandle handle) const;

		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

		void AddToLoadedAssets(Ref<Asset> asset, bool persistent = false);
		void UnloadAsset(AssetHandle& handle);

		const AssetMap& GetTempAssetMap() const { return m_LoadedAssets; }
		const AssetMap& GetPersistentAssetMap() const { return m_LoadedPersistentAssets; }

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();
	private:
		AssetRegistry m_AssetRegistry;
		AssetFileRegistry m_AssetFileRegistry;
		AssetMap m_LoadedAssets;
		AssetMap m_LoadedPersistentAssets;
	};
}