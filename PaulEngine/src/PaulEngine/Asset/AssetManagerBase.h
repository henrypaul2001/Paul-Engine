#pragma once
#include "Asset.h"
#include <unordered_map>

namespace PaulEngine
{
	using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

	// Interface API for different asset managers such as runtime assets vs editor assets
	class AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
		virtual bool IsAssetRegistered(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
		virtual AssetType GetAssetType(AssetHandle handle) const = 0;
	};
}