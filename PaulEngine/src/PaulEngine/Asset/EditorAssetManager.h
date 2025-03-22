#pragma once
#include "AssetManagerBase.h"
#include "Asset.h"

#include <unordered_map>

namespace PaulEngine
{
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;
	class EditorAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const override;

		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;

		const AssetMetadata& GetMetadata(AssetHandle handle) const;

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};
}