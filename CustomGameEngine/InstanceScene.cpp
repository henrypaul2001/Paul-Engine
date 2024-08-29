#include "InstanceScene.h"
#include "GameInputManager.h"
namespace Engine {
	InstanceScene::InstanceScene(SceneManager* sceneManager) : Scene(sceneManager, "InstanceScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	InstanceScene::~InstanceScene()
	{

	}

	void InstanceScene::ChangePostProcessEffect()
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

	void InstanceScene::Update()
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

	void InstanceScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void InstanceScene::Close()
	{

	}

	void InstanceScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void InstanceScene::keyUp(int key)
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

	void InstanceScene::keyDown(int key)
	{

	}

	void InstanceScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(6.0f, 6.0f, 8.0f);
		directional->Specular = glm::vec3(0.0f);
		directional->ShadowProjectionSize = 70.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Material* textured = new Material();
		textured->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, false));
		textured->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		textured->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		textured->shininess = 5.0f;

		PBRMaterial* bricks = new PBRMaterial();
		bricks->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
		bricks->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
		bricks->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
		bricks->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
		bricks->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
		bricks->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
		bricks->height_scale = -0.1;

		PBRMaterial* gold = new PBRMaterial();
		gold->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));

		Entity* baseInstance = new Entity("Base Instance");
		baseInstance->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		baseInstance->AddComponent(new ComponentGeometry(MODEL_CUBE, true, true));
		baseInstance->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(gold);
		entityManager->AddEntity(baseInstance);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(0.0f, 0.0f, -2.0f));
		pointLight->AddComponent(new ComponentLight(POINT));
		pointLight->GetLightComponent()->Colour = glm::vec3(0.8f, 0.15f, 0.25f);
		entityManager->AddEntity(pointLight);

		int xNum = 5;
		int yNum = 5;
		int zNum = 5;

		float originX = float(-xNum) / 2.0f;
		float originY = float(-yNum) / 2.0f;
		float originZ = -10.0f;

		float xDistance = 2.25f;
		float yDistance = 2.25f;
		float zDistance = -2.25f;

		int count = 0;
		for (int i = 0; i < yNum; i++) {
			for (int j = 0; j < xNum; j++) {
				for (int k = 0; k < zNum; k++) {
					std::string name = std::string("Box ") + std::string(std::to_string(count));
					Entity* box = new Entity(name);
					box->AddComponent(new ComponentTransform(originX + (j * xDistance), originY + (i * yDistance), originZ + (k * zDistance)));
					entityManager->AddEntity(box);
					baseInstance->GetGeometryComponent()->AddNewInstanceSource(box);
					count++;
					std::cout << "box " << count << " created" << std::endl;
				}
			}
		}

		/*
		Entity* baseInstance2 = new Entity("Base Instance 2");
		baseInstance2->AddComponent(new ComponentTransform(0.0f, 0.0f, 2.5f));
		baseInstance2->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		entityManager->AddEntity(baseInstance2);

		for (int i = 0; i < yNum; i++) {
			for (int j = 0; j < xNum; j++) {
				for (int k = 0; k < zNum; k++) {
					std::string name = std::string("Box ") + std::string(std::to_string(count));
					Entity* box = new Entity(name);
					box->AddComponent(new ComponentTransform(originX + (j * xDistance), originY + (i * yDistance), -originZ + (k * -zDistance)));
					entityManager->AddEntity(box);
					baseInstance2->GetGeometryComponent()->AddNewInstanceSource(box);
					count++;
					std::cout << "box " << count << " created" << std::endl;
				}
			}
		}
		*/
		std::cout << count << " box instances created" << std::endl;

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
	}

	void InstanceScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
	}
}