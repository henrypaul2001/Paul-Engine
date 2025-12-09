#include "AssetPreviewPanel.h"
#include "PaulEngine/Asset/AssetManager.h"
namespace PaulEngine
{
	std::unordered_map<AssetType, AssetPreviewContextAddedFunc> AssetPreviewPanel::OnContextAddedFunctionMap =
	{
		{ AssetType::None, NullAssetSelected },
		{ AssetType::Material, MaterialAssetSelected },
		{ AssetType::Texture2D, Texture2DAssetSelected },
		{ AssetType::TextureCubemap, TextureCubemapAssetSelected }
	};

	AssetPreviewPanel::AssetPreviewPanel()
	{
	}

	void AssetPreviewPanel::OnImGuiRender()
	{
		// Render scene with a generic renderer

		// Display render in ImGui::Image
	}

	void AssetPreviewPanel::SetContext(AssetHandle previewAsset)
	{
		AssetType previewType = AssetManager::GetAssetType(previewAsset);
		auto it = OnContextAddedFunctionMap.find(previewType);
		if (it != OnContextAddedFunctionMap.end())
		{
			m_PreviewAsset = previewAsset;
			it->second(m_PreviewScene, previewAsset);
		}
		else
		{
			PE_CORE_ERROR("No asset preview function found for type '{0}'", AssetTypeToString(previewType));
		}
	}

	void AssetPreviewPanel::NullAssetSelected(Ref<Scene> previewScene, AssetHandle emptyHandle)
	{
		PE_CORE_INFO("Null asset");
	}

	void AssetPreviewPanel::MaterialAssetSelected(Ref<Scene> previewScene, AssetHandle materialHandle)
	{
		PE_CORE_INFO("Material asset");
	}

	void AssetPreviewPanel::Texture2DAssetSelected(Ref<Scene> previewScene, AssetHandle textureHandle)
	{
		PE_CORE_INFO("Texture2D asset");
	}

	void AssetPreviewPanel::TextureCubemapAssetSelected(Ref<Scene> previewScene, AssetHandle textureHandle)
	{
		PE_CORE_INFO("TextureCubemap asset");
	}
}