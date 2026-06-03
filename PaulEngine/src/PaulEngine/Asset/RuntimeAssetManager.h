#pragma once
#include "AssetManagerBase.h"
#include "EditorAssetManager.h"

namespace PaulEngine
{
	// TODO: runtime assets
	using RuntimeAssetManager = EditorAssetManager;
	//class RuntimeAssetManager : public AssetManagerBase
	//{
	//public:
	//	virtual Ref<Asset> GetAsset(AssetHandle handle) override;
	//
	//	virtual bool IsAssetRegistered(AssetHandle handle) const override;
	//	virtual bool IsAssetLoaded(AssetHandle handle) const override;
	//};
}