#pragma once
#include "AssetManagerBase.h"

namespace PaulEngine
{
	class RuntimeAssetManager : public AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;

		virtual bool IsAssetRegistered(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
	};
}