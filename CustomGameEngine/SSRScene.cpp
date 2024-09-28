#include "SSRScene.h"
#include "SystemUIRender.h"
#include "SystemUIMouseInteraction.h"
#include "GameInputManager.h"
#include "SystemAnimatedGeometryAABBGeneration.h"
#include "SystemSkeletalAnimationUpdater.h"
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

		ResourceManager::GetInstance()->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");
	}

	SSRScene::~SSRScene()
	{

	}

	void SSRScene::ChangePostProcessEffect()
	{
		SystemRender* renderSystem = dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS));
		unsigned int currentEffect = renderSystem->GetPostProcess();
		unsigned int nextEffect;
		if (currentEffect == 8u) {
			nextEffect = 0u;
		}
		else {
			nextEffect = currentEffect + 1;
		}

		dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS))->SetPostProcess((PostProcessingEffect)nextEffect);
	}

	void SSRScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);

		float time = (float)glfwGetTime();

		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) {
			targetFPSPercentage = 1.0f;
		}

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		Entity* canvas = entityManager->FindEntity("Canvas");
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		BVHTree* geometryBVH = collisionManager->GetBVHTree();
		SystemFrustumCulling* culling = dynamic_cast<SystemFrustumCulling*>(systemManager->FindSystem(SYSTEM_FRUSTUM_CULLING, UPDATE_SYSTEMS));

		unsigned int meshCount = culling->GetTotalMeshes();
		unsigned int visibleMeshes = culling->GetVisibleMeshes();
		unsigned int nodeCount = geometryBVH->GetNodeCount();
		unsigned int aabbTests = culling->GetTotalAABBTests();

		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[6])->SetText("Mesh count: " + std::to_string(meshCount));
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[7])->SetText("     - Visible: " + std::to_string(visibleMeshes));
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[8])->SetText("BVHN count: " + std::to_string(nodeCount));
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[9])->SetText("AABB Tests: " + std::to_string(aabbTests));
	}

	void SSRScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void SSRScene::Close()
	{

	}

	void SSRScene::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void SSRScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			Entity* canvas = entityManager->FindEntity("Canvas");

			canvas->GetUICanvasComponent()->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void SSRScene::keyDown(int key)
	{

	}

	void SSRScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Colour = glm::vec3(3.9f, 3.1f, 7.5f);
		directional->Ambient = directional->Colour * 0.1f;
		directional->Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional->MinShadowBias = 0.0f;
		directional->MaxShadowBias = 0.003f;
		directional->DirectionalLightDistance = 20.0f;
		directional->ShadowProjectionSize = 20.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

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
		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -0.5f, 0.0f));
		floor->GetTransformComponent()->SetScale(glm::vec3(25.0f, 0.5, 25.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		floor->GetGeometryComponent()->ApplyMaterialToModel(stoneTiles);
		entityManager->AddEntity(floor);

		Entity* pillar = new Entity("Pillar");
		pillar->AddComponent(new ComponentTransform(2.5, 5.0f, -5.0f));
		pillar->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		pillar->GetTransformComponent()->SetScale(glm::vec3(1.0f, 5.0f, 1.0f));
		pillar->GetGeometryComponent()->ApplyMaterialToModel(marbleTile);
		entityManager->AddEntity(pillar);

		Entity* pillarClone = pillar->Clone();
		pillarClone->GetTransformComponent()->SetPosition(pillar->GetTransformComponent()->GetWorldPosition() + glm::vec3(-4.5f, 0.0f, -2.5f));

		Entity* goldSheet = new Entity("Gold Sheet");
		goldSheet->AddComponent(new ComponentTransform(0.0f, 0.0f, 1.0f));
		goldSheet->GetTransformComponent()->SetScale(glm::vec3(3.5f, 0.025f, 3.5f));
		goldSheet->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		goldSheet->GetGeometryComponent()->ApplyMaterialToModel(gold);
		entityManager->AddEntity(goldSheet);

		SkeletalAnimation* vampireDanceAnim = resources->LoadAnimation("Models/vampire/dancing_vampire.dae");
		Entity* vampire = new Entity("Vampire");
		vampire->AddComponent(new ComponentTransform(0.0f, 0.0f, 1.0f));
		vampire->AddComponent(new ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
		vampire->AddComponent(new ComponentAnimator(vampireDanceAnim));
		entityManager->AddEntity(vampire);

		Entity* vent = new Entity("Vent");
		vent->AddComponent(new ComponentTransform(-5.0f, 1.0f, 0.0f));
		vent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		vent->GetGeometryComponent()->ApplyMaterialToModel(metalVent);
		entityManager->AddEntity(vent);

		Entity* block = vent->Clone();
		block->GetTransformComponent()->SetPosition(vent->GetTransformComponent()->GetWorldPosition() + glm::vec3(0.05f, 2.0f, -0.1f));
		block->GetGeometryComponent()->ApplyMaterialToModel(metalGrid);

		Entity* woodPlane = new Entity("Wood Plane");
		woodPlane->AddComponent(new ComponentTransform(8.5f, 2.0f, 1.0f));
		woodPlane->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		woodPlane->GetGeometryComponent()->ApplyMaterialToModel(darkWood);
		woodPlane->GetTransformComponent()->SetScale(glm::vec3(5.0f, 4.0f, 1.0f));
		woodPlane->GetTransformComponent()->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), -135.0f);
		entityManager->AddEntity(woodPlane);

		Entity* pointLight = new Entity("Light");
		pointLight->AddComponent(new ComponentTransform(6.0f, 2.5f, -1.0f));
		pointLight->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		pointLight->GetTransformComponent()->SetScale(0.25f);
		ComponentLight* light = new ComponentLight(POINT);
		light->Colour = glm::vec3(50.0, 50.0, 50.0);
		light->CastShadows = false;
		pointLight->AddComponent(light);
		entityManager->AddEntity(pointLight);

#pragma endregion

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(25.0f, 135.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(25.0f, 10.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Resolution: ") + std::to_string(SCR_WIDTH) + " X " + std::to_string(SCR_HEIGHT), glm::vec2(25.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Shadow res: ") + std::to_string(renderManager->ShadowWidth()) + " X " + std::to_string(renderManager->ShadowHeight()), glm::vec2(25.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		std::string renderPipeline = "DEFERRED";
		if (renderManager->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
			renderPipeline = "FORWARD";
		}
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("G Pipeline: ") + renderPipeline, glm::vec2(25.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

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
		canvas->GetUICanvasComponent()->AddUIElement(geoDebugText);
		canvas->GetUICanvasComponent()->AddUIElement(meshCountText);
		canvas->GetUICanvasComponent()->AddUIElement(visibleCountText);
		canvas->GetUICanvasComponent()->AddUIElement(bvhCountText);
		canvas->GetUICanvasComponent()->AddUIElement(aabbTestCountText);
		entityManager->AddEntity(canvas);
#pragma endregion
	}

	void SSRScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIMouseInteraction(inputManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemSkeletalAnimationUpdater(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemAnimatedGeometryAABBGeneration(), UPDATE_SYSTEMS);
	}
}