#pragma once
#include "Asset.h"
#include <map>

namespace PaulEngine
{
	using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

	// Interface API for different asset managers such as runtime assets vs editor assets
	class AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) const = 0;
		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
	};
}