#include "SSRScene.h"
#include "GameInputManager.h"
#include "UIText.h"
namespace Engine {
	SSRScene::SSRScene(SceneManager* sceneManager) : Scene(sceneManager, "SSRScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(400.0f);
		renderManager->GetRenderParams()->SetBloomPasses(10);
		renderManager->GetRenderParams()->SetSSAOSamples(32);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);

		resources->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");

		const char* envMap = "Textures/Environment Maps/sky.hdr";
		resources->LoadHDREnvironmentMap(envMap, true);
		renderManager->SetEnvironmentMap(envMap);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
	}

	SSRScene::~SSRScene()
	{

	}

	void SSRScene::ChangePostProcessEffect()
	{
		SystemRender& renderSystem = renderManager->GetRenderPipeline()->GetRenderSystem();
		unsigned int currentEffect = renderSystem.GetPostProcess();
		unsigned int nextEffect;
		if (currentEffect == 8u) {
			nextEffect = 0u;
		}
		else {
			nextEffect = currentEffect + 1;
		}

		renderSystem.SetPostProcess((PostProcessingEffect)nextEffect);
	}

	void SSRScene::Update()
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

	void SSRScene::Render()
	{
		Scene::Render();
	}

	void SSRScene::Close() {}

	void SSRScene::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		RegisterAllDefaultSystems();
		CreateEntities();
	}

	void SSRScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
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

	void SSRScene::keyDown(int key)
	{

	}

	void SSRScene::CreateEntities()
	{
		EntityNew* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Colour = glm::vec3(3.9f, 3.1f, 7.5f);
		directional.Ambient = directional.Colour * 0.1f;
		directional.Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional.MinShadowBias = 0.0f;
		directional.MaxShadowBias = 0.003f;
		directional.DirectionalLightDistance = 20.0f;
		directional.ShadowProjectionSize = 20.0f;
		ecs.AddComponent(dirLight->ID(), directional);

#pragma region Materials
		PBRMaterial* metalVent = new PBRMaterial();
		metalVent->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/metalVent/albedo.png", TEXTURE_ALBEDO, true));
		metalVent->normalMaps.push_back(resources->LoadTexture("Materials/PBR/metalVent/normal.png", TEXTURE_NORMAL, false));
		metalVent->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/metalVent/metal.png", TEXTURE_METALLIC, false));
		metalVent->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/metalVent/roughness.png", TEXTURE_ROUGHNESS, false));
		metalVent->aoMaps.push_back(resources->LoadTexture("Materials/PBR/metalVent/ao.png", TEXTURE_AO, false));
		//metalVent->heightMaps.push_back(resources->LoadTexture("Materials/PBR/metalVent/height.png", TEXTURE_DISPLACE, false));
		metalVent->height_scale = -0.1;
		//metalVent->textureScaling = glm::vec2(10.0f);
		resources->AddMaterial("Metal Vent", metalVent);

		PBRMaterial* stoneTiles = new PBRMaterial();
		stoneTiles->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/stoneTiles/albedo.png", TEXTURE_ALBEDO, true));
		stoneTiles->normalMaps.push_back(resources->LoadTexture("Materials/PBR/stoneTiles/normal.png", TEXTURE_NORMAL, false));
		stoneTiles->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/stoneTiles/metal.png", TEXTURE_METALLIC, false));
		stoneTiles->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/stoneTiles/roughness.png", TEXTURE_ROUGHNESS, false));
		stoneTiles->aoMaps.push_back(resources->LoadTexture("Materials/PBR/stoneTiles/ao.png", TEXTURE_AO, false));
		//stoneTiles->heightMaps.push_back(resources->LoadTexture("Materials/PBR/stoneTiles/height.png", TEXTURE_DISPLACE, false));
		stoneTiles->height_scale = -0.1;
		stoneTiles->textureScaling = glm::vec2(10.0f);
		resources->AddMaterial("Stone Tiles", stoneTiles);

		PBRMaterial* marbleTile = new PBRMaterial();
		marbleTile->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/marbleTile/albedo.png", TEXTURE_ALBEDO, true));
		marbleTile->normalMaps.push_back(resources->LoadTexture("Materials/PBR/marbleTile/normal.png", TEXTURE_NORMAL, false));
		marbleTile->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/marbleTile/metal.png", TEXTURE_METALLIC, false));
		marbleTile->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/marbleTile/roughness.png", TEXTURE_ROUGHNESS, false));
		marbleTile->aoMaps.push_back(resources->LoadTexture("Materials/PBR/marbleTile/ao.png", TEXTURE_AO, false));
		//marbleTile->heightMaps.push_back(resources->LoadTexture("Materials/PBR/marbleTile/height.png", TEXTURE_DISPLACE, false));
		marbleTile->height_scale = -0.1;
		marbleTile->textureScaling = glm::vec2(1.0f, 5.0f);
		resources->AddMaterial("Marble Tile", marbleTile);

		PBRMaterial* darkWood = new PBRMaterial();
		darkWood->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/darkWood/albedo.png", TEXTURE_ALBEDO, true));
		darkWood->normalMaps.push_back(resources->LoadTexture("Materials/PBR/darkWood/normal.png", TEXTURE_NORMAL, false));
		darkWood->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/darkWood/metal.png", TEXTURE_METALLIC, false));
		darkWood->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/darkWood/roughness.png", TEXTURE_ROUGHNESS, false));
		darkWood->aoMaps.push_back(resources->LoadTexture("Materials/PBR/darkWood/ao.png", TEXTURE_AO, false));
		//darkWood->heightMaps.push_back(resources->LoadTexture("Materials/PBR/darkWood/height.png", TEXTURE_DISPLACE, false));
		darkWood->height_scale = -0.1;
		darkWood->textureScaling = glm::vec2(10.0f);
		resources->AddMaterial("Dark Wood", darkWood);

		PBRMaterial* metalGrid = new PBRMaterial();
		metalGrid->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/metalGrid/albedo.png", TEXTURE_ALBEDO, true));
		metalGrid->normalMaps.push_back(resources->LoadTexture("Materials/PBR/metalGrid/normal.png", TEXTURE_NORMAL, false));
		metalGrid->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/metalGrid/metal.png", TEXTURE_METALLIC, false));
		metalGrid->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/metalGrid/roughness.png", TEXTURE_ROUGHNESS, false));
		metalGrid->aoMaps.push_back(resources->LoadTexture("Materials/PBR/metalGrid/ao.png", TEXTURE_AO, false));
		//metalGrid->textureScaling = glm::vec2(10.0f);
		resources->AddMaterial("Metal Grid", metalGrid);

		PBRMaterial* gold = new PBRMaterial();
		gold->baseColourMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));
		resources->AddMaterial("Gold", gold);
#pragma endregion

#pragma region Scene
		EntityNew* floor = ecs.New("Floor");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(0.0f, -0.5f, 0.0f));
		transform->SetScale(glm::vec3(25.0f, 0.5, 25.0f));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE, true));
		ecs.GetComponent<ComponentGeometry>(floor->ID())->ApplyMaterialToModel(stoneTiles);

		EntityNew* pillar = ecs.New("Pillar");
		transform = ecs.GetComponent<ComponentTransform>(pillar->ID());
		transform->SetPosition(glm::vec3(2.5, 5.0f, -5.0f));
		ecs.AddComponent(pillar->ID(), ComponentGeometry(MODEL_CUBE, true));
		transform->SetScale(glm::vec3(1.0f, 5.0f, 1.0f));
		ecs.GetComponent<ComponentGeometry>(pillar->ID())->ApplyMaterialToModel(marbleTile);

		EntityNew* pillarClone = ecs.Clone(pillar->ID());
		transform = ecs.GetComponent<ComponentTransform>(pillarClone->ID());
		transform->SetPosition(transform->GetWorldPosition() + glm::vec3(-4.5f, 0.0f, -2.5f));

		EntityNew* goldSheet = ecs.New("Gold Sheet");
		transform = ecs.GetComponent<ComponentTransform>(goldSheet->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		transform->SetScale(glm::vec3(3.5f, 0.025f, 3.5f));
		ecs.AddComponent(goldSheet->ID(), ComponentGeometry(MODEL_CUBE, true));
		ecs.GetComponent<ComponentGeometry>(goldSheet->ID())->ApplyMaterialToModel(gold);

		SkeletalAnimation* vampireDanceAnim = resources->LoadAnimation("Models/vampire/dancing_vampire.dae");
		EntityNew* vampire = ecs.New("Vampire");
		transform = ecs.GetComponent<ComponentTransform>(vampire->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		ecs.AddComponent(vampire->ID(), ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
		ecs.AddComponent(vampire->ID(), ComponentAnimator(vampireDanceAnim));

		EntityNew* vent = ecs.New("Vent");
		transform = ecs.GetComponent<ComponentTransform>(vent->ID());
		transform->SetPosition(glm::vec3(-5.0f, 1.0f, 0.0f));
		ecs.AddComponent(vent->ID(), ComponentGeometry(MODEL_CUBE, true));
		ecs.GetComponent<ComponentGeometry>(vent->ID())->ApplyMaterialToModel(metalVent);

		EntityNew* block = ecs.Clone(vent->ID());
		transform = ecs.GetComponent<ComponentTransform>(block->ID());
		transform->SetPosition(transform->GetWorldPosition() + glm::vec3(0.05f, 2.0f, -0.1f));
		ecs.GetComponent<ComponentGeometry>(block->ID())->ApplyMaterialToModel(metalGrid);

		EntityNew* woodPlane = ecs.New("Wood Plane");
		transform = ecs.GetComponent<ComponentTransform>(woodPlane->ID());
		transform->SetPosition(glm::vec3(8.5f, 2.0f, 1.0f));
		ecs.AddComponent(woodPlane->ID(), ComponentGeometry(MODEL_PLANE, true));
		ecs.GetComponent<ComponentGeometry>(woodPlane->ID())->ApplyMaterialToModel(darkWood);
		transform->SetScale(glm::vec3(5.0f, 4.0f, 1.0f));
		transform->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), -135.0f);

		EntityNew* pointLight = ecs.New("Light");
		transform = ecs.GetComponent<ComponentTransform>(pointLight->ID());
		transform->SetPosition(glm::vec3(6.0f, 2.5f, -1.0f));
		ecs.AddComponent(pointLight->ID(), ComponentGeometry(MODEL_SPHERE, true));
		transform->SetScale(0.25f);
		ComponentLight light = ComponentLight(POINT);
		light.Colour = glm::vec3(50.0, 50.0, 50.0);
		light.CastShadows = true;
		ecs.AddComponent(pointLight->ID(), light);

		EntityNew* testEnvironment = ecs.New("Test Environment");
		ecs.AddComponent(testEnvironment->ID(), ComponentGeometry("Models/PBR/SSRTestEnvironment/ssrTestEnvironment.obj", true));

		Model* testScene = ecs.GetComponent<ComponentGeometry>(testEnvironment->ID())->GetModel();
		testScene->meshes[0]->GetMaterial()->textureScaling = glm::vec2(25.0f, 25.0f);
		testScene->meshes[1]->GetMaterial()->textureScaling = glm::vec2(3.3f, 0.6f);
		testScene->meshes[6]->GetMaterial()->textureScaling = glm::vec2(10.0f, 10.0f);
		testScene->meshes[7]->GetMaterial()->textureScaling = glm::vec2(10.0f, 10.0f);
		testScene->meshes[8]->GetMaterial()->textureScaling = glm::vec2(10.0f, 10.0f);
		testScene->meshes[9]->GetMaterial()->textureScaling = glm::vec2(10.0f, 10.0f);
		testScene->meshes[10]->GetMaterial()->textureScaling = glm::vec2(2.5f, 2.5f);

#pragma endregion

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
}