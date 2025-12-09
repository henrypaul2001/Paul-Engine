#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Scene/Scene.h"

namespace PaulEngine
{
	using AssetPreviewContextAddedFunc = std::function<void(Ref<Scene>, AssetHandle)>;
	class AssetPreviewPanel
	{
	public:
		AssetPreviewPanel();

		void OnImGuiRender();
		void SetContext(AssetHandle previewAsset);
	
		static std::unordered_map<AssetType, AssetPreviewContextAddedFunc> OnContextAddedFunctionMap;

		static void NullAssetSelected(Ref<Scene> previewScene, AssetHandle emptyHandle);
		static void MaterialAssetSelected(Ref<Scene> previewScene, AssetHandle materialHandle);
		static void Texture2DAssetSelected(Ref<Scene> previewScene, AssetHandle textureHandle);
		static void TextureCubemapAssetSelected(Ref<Scene> previewScene, AssetHandle textureHandle);

	private:
		Ref<Scene> m_PreviewScene;
		AssetHandle m_PreviewAsset = 0;
	};
}