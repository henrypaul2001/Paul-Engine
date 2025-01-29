#include "SponzaScene.h"
#include "GameInputManager.h"
#include "SystemUIRender.h"
#include "SystemUIMouseInteraction.h"
namespace Engine {
	SponzaScene::SponzaScene(SceneManager* sceneManager) : Scene(sceneManager, "SponzaScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);

		renderManager->GetRenderParams()->SetSSAOSamples(32);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);
		resources->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);

		renderManager->SetEnvironmentMap("Textures/Environment Maps/sky.hdr");
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
		resources->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");

		SetupScene();
	}

	SponzaScene::~SponzaScene() {}

	void SponzaScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void SponzaScene::keyUp(int key)
	{
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			EntityNew* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void SponzaScene::keyDown(int key)
	{

	}

	void SponzaScene::CreateEntities()
	{
		PBRMaterial* gold = new PBRMaterial();
		gold->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(resources->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(resources->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));
		resources->AddMaterial("gold", gold);

		EntityNew* dirLight = ecs.New("Directional Light");
		ComponentLight light = ComponentLight(DIRECTIONAL);
		light.Direction = glm::vec3(0.35f, -0.6f, 0.0f);
		light.Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		light.Ambient = glm::vec3(0.01f, 0.01f, 0.05f) * 0.0f;
		light.ShadowProjectionSize = 30.0f;
		ecs.AddComponent(dirLight->ID(), light);

		resources->SetModelLoaderTextureTranslation(TEXTURE_NORMAL, aiTextureType_NORMALS);
		resources->SetModelLoaderTextureTranslation(TEXTURE_METALLIC, aiTextureType_METALNESS);
		resources->SetModelLoaderTextureTranslation(TEXTURE_ROUGHNESS, aiTextureType_DIFFUSE_ROUGHNESS);
		resources->SetModelLoaderTextureTranslation(TEXTURE_AO, aiTextureType_AMBIENT_OCCLUSION);
		resources->SetModelLoaderTextureTranslation(TEXTURE_HEIGHT, aiTextureType_HEIGHT);

		EntityNew* sponza = ecs.New("Sponza");
		ecs.AddComponent(sponza->ID(), ComponentGeometry("Models/PBR/newSponza/base/NewSponza_Main_glTF_003.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));

		EntityNew* curtains = ecs.New("Curtains");
		ecs.AddComponent(curtains->ID(), ComponentGeometry("Models/PBR/newSponza/curtains/NewSponza_Curtains_glTF.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));

		EntityNew* ivy = ecs.New("Ivy");
		ecs.AddComponent(ivy->ID(), ComponentGeometry("Models/PBR/newSponza/ivy/NewSponza_IvyGrowth_glTF.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));

		EntityNew* trees = ecs.New("Trees");
		ecs.AddComponent(trees->ID(), ComponentGeometry("Models/PBR/newSponza/trees/NewSponza_CypressTree_glTF.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));
		ComponentGeometry* geometry = ecs.GetComponent<ComponentGeometry>(trees->ID());
		geometry->SetCulling(false, GL_BACK);
		geometry->GetModel()->meshes[0]->GetMaterial()->SetUseColourMapAsAlpha(false);
		geometry->GetModel()->meshes[1]->GetMaterial()->SetUseColourMapAsAlpha(true);
		geometry->GetModel()->meshes[2]->GetMaterial()->SetUseColourMapAsAlpha(false);

		resources->ResetModelLoaderTextureTranslationsToDefault();

		EntityNew* pointLight1 = ecs.New("Point Light 1");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(pointLight1->ID());
		transform->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
		ecs.AddComponent(pointLight1->ID(), ComponentGeometry(MODEL_SPHERE, true));
		geometry = ecs.GetComponent<ComponentGeometry>(pointLight1->ID());
		geometry->ApplyMaterialToModel(gold);
		geometry->CastShadows(false);
		transform->SetScale(0.25f);
		light = ComponentLight(SPOT);
		light.Colour = glm::vec3(224.0f, 222.0f, 164.0f) * 0.3f;
		light.Ambient = glm::vec3(0.0f);
		light.CastShadows = false;
		ecs.AddComponent(pointLight1->ID(), light);

		EntityNew* pointLight2 = ecs.New("Point Light 2");
		transform = ecs.GetComponent<ComponentTransform>(pointLight2->ID());
		transform->SetPosition(glm::vec3(-10.0f, 7.5f, 0.0f));
		ecs.AddComponent(pointLight2->ID(), ComponentGeometry(MODEL_SPHERE, true));
		geometry = ecs.GetComponent<ComponentGeometry>(pointLight2->ID());
		geometry->ApplyMaterialToModel(gold);
		geometry->CastShadows(false);
		transform->SetScale(0.25f);
		light = ComponentLight(POINT);
		light.Colour = glm::vec3(224.0f, 222.0f, 164.0f) * 0.7f;
		light.Ambient = glm::vec3(0.0f);
		light.CastShadows = false;
		ecs.AddComponent(pointLight2->ID(), light);

		EntityNew* pointLight3 = ecs.New("Point Light 3");
		transform = ecs.GetComponent<ComponentTransform>(pointLight3->ID());
		transform->SetPosition(glm::vec3(16.0f, 1.5f, -1.0f));
		ecs.AddComponent(pointLight3->ID(), ComponentGeometry(MODEL_SPHERE, true));
		geometry = ecs.GetComponent<ComponentGeometry>(pointLight3->ID());
		geometry->ApplyMaterialToModel(gold);
		geometry->CastShadows(false);
		transform->SetScale(0.25f);
		light = ComponentLight(POINT);
		light.Colour = glm::vec3(224.0f, 222.0f, 164.0f) * 0.7f;
		light.Ambient = glm::vec3(0.0f);
		light.CastShadows = false;
		ecs.AddComponent(pointLight3->ID(), light);

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
		EntityNew* uiCanvas = ecs.New("Canvas");
		ecs.AddComponent(uiCanvas->ID(), ComponentUICanvas(SCREEN_SPACE));
		ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());
		canvas->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(25.0f, 135.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(25.0f, 10.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->AddUIElement(new UIText(std::string("Resolution: ") + std::to_string(SCR_WIDTH) + " X " + std::to_string(SCR_HEIGHT), glm::vec2(25.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));
		canvas->AddUIElement(new UIText(std::string("Shadow res: ") + std::to_string(renderManager->ShadowWidth()) + " X " + std::to_string(renderManager->ShadowHeight()), glm::vec2(25.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		const std::string renderPipeline = renderManager->GetRenderPipeline()->PipelineName();
		canvas->AddUIElement(new UIText(renderPipeline, glm::vec2(25.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		// Geometry debug UI
		UIBackground geometryDebugBackground;
		geometryDebugBackground.Colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		geometryDebugBackground.LeftRightUpDownExtents = glm::vec4(0.01f, 0.225f, 0.05f, 0.8f);
		geometryDebugBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		geometryDebugBackground.BorderThickness = 0.01f;
		geometryDebugBackground.Bordered = true;

		UIText* geoDebugText = new UIText(std::string("Geometry Debug"), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 135.0f), glm::vec2(0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		geoDebugText->SetBackground(geometryDebugBackground);
		geoDebugText->UseBackground(true);
		geoDebugText->SetActive(false);
		UIText* meshCountText = new UIText(std::string("Mesh count: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		meshCountText->SetActive(false);
		UIText* visibleCountText = new UIText(std::string("     - Visible: "), glm::vec2((SCR_WIDTH / 2.0f) - 145.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		visibleCountText->SetActive(false);
		UIText* bvhCountText = new UIText(std::string("BVHN count: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		bvhCountText->SetActive(false);
		UIText* aabbTestCountText = new UIText(std::string("AABB Tests: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 15.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		aabbTestCountText->SetActive(false);
		canvas->AddUIElement(geoDebugText);
		canvas->AddUIElement(meshCountText);
		canvas->AddUIElement(visibleCountText);
		canvas->AddUIElement(bvhCountText);
		canvas->AddUIElement(aabbTestCountText);
#pragma endregion
	}

	void SponzaScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}

	void SponzaScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		float time = (float)glfwGetTime();
		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) { targetFPSPercentage = 1.0f; }

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		EntityNew* canvasEntity = ecs.Find("Canvas");
		ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(canvasEntity->ID());
		dynamic_cast<UIText*>(canvas->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(canvas->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		BVHTree* geometryBVH = collisionManager->GetBVHTree();

		const unsigned int meshCount = frustumCulling.GetTotalMeshes();
		const unsigned int visibleMeshes = frustumCulling.GetVisibleMeshes();
		const unsigned int nodeCount = geometryBVH->GetNodeCount();
		const unsigned int aabbTests = frustumCulling.GetTotalAABBTests();

		dynamic_cast<UIText*>(canvas->UIElements()[6])->SetText("Mesh count: " + std::to_string(meshCount));
		dynamic_cast<UIText*>(canvas->UIElements()[7])->SetText("     - Visible: " + std::to_string(visibleMeshes));
		dynamic_cast<UIText*>(canvas->UIElements()[8])->SetText("BVHN count: " + std::to_string(nodeCount));
		dynamic_cast<UIText*>(canvas->UIElements()[9])->SetText("AABB Tests: " + std::to_string(aabbTests));

		systemManager.ActionSystems();
	}

	void SponzaScene::Render()
	{
		Scene::Render();
	}

	void SponzaScene::Close() {}
}