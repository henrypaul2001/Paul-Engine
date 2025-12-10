#include "AssetPreviewPanel.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Scene/Components.h"

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
		m_PreviewScene = CreateRef<Scene>();
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

			// Clear the preview scene and create a basic template that can be expanded upon further by the AssetPreviewContextAddedFunc call
			m_PreviewScene->Clear();
			Entity cameraEntity = m_PreviewScene->CreateEntity("Camera");
			ComponentCamera& camComponent = cameraEntity.AddComponent<ComponentCamera>();
			camComponent.Camera = SceneCamera(SCENE_CAMERA_PERSPECTIVE);

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
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		
		// Set up a preview mesh with material
		Entity previewMesh = previewScene->CreateEntity("Mesh");
		previewMesh.GetComponent<ComponentTransform>().SetLocalPosition(glm::vec3(0.0f, 0.0f, -2.0f));
		ComponentMeshRenderer& meshComponent = previewMesh.AddComponent<ComponentMeshRenderer>();

		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());
		meshComponent.MeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultSphere.pmesh", true);
		ComponentMeshRenderer::SetMaterial(previewMesh, materialHandle);
	
		// Add lighting and render volume
		Entity pointLightEntity = previewScene->CreateEntity("Point Light");
		pointLightEntity.GetComponent<ComponentTransform>().SetLocalPosition(glm::vec3(-0.5f, 0.5f, 0.0f));
		ComponentPointLight& pointLightComponent = pointLightEntity.AddComponent<ComponentPointLight>();
		pointLightComponent.CastShadows = false;
		pointLightComponent.Diffuse = glm::vec3(1.0f);
		pointLightComponent.Specular = glm::vec3(1.0f);
		pointLightComponent.Ambient = glm::vec3(0.25f);
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