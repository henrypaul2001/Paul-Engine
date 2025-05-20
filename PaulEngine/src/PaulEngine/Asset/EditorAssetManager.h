#pragma once
#include "AssetManagerBase.h"
#include "Asset.h"

#include <unordered_map>

namespace PaulEngine
{
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	using AssetSourceRegistry = std::unordered_map<std::filesystem::path, AssetHandle>;
	class EditorAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;
		virtual AssetType GetAssetType(AssetHandle handle) const override;
		virtual const AssetMetadata& GetMetadata(AssetHandle handle) const override;

		bool IsAssetTempLoaded(AssetHandle handle) const;
		bool IsAssetPersistentLoaded(AssetHandle handle) const;

		virtual bool IsAssetRegistered(AssetHandle handle) const override;
		bool IsSourceFileRegistered(const std::filesystem::path& filepath) const;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual bool IsAssetProcedural(AssetHandle handle) const override;

		virtual void UnloadAsset(AssetHandle& handle) override;
		virtual void ReleaseTempAssets() override;
		virtual void RegisterAsset(AssetHandle handle, AssetMetadata metadata) override;

		AssetHandle ImportAssetFromFile(const std::filesystem::path& filepath, const bool persistent);

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

		const AssetMap& GetTempAssetMap() const { return m_TempAssets; }
		const AssetMap& GetPersistentAssetMap() const { return m_PersistentAssets; }
		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

	protected:
		virtual void AddToLoadedAssets(Ref<Asset> asset, bool persistent = false) override;

	private:
		AssetRegistry m_AssetRegistry;
		AssetSourceRegistry m_SourceFileRegistry;
		AssetMap m_TempAssets;
		AssetMap m_PersistentAssets;
	};
}