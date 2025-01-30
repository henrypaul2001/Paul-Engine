#include "InstanceScene.h"
#include "GameInputManager.h"
#include "UIText.h"
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

	void InstanceScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		float time = (float)glfwGetTime();
		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) { targetFPSPercentage = 1.0f; }

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		Entity* canvasEntity = ecs.Find("Canvas");
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

	void InstanceScene::Render()
	{
		Scene::Render();
	}

	void InstanceScene::Close() {}

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
			Entity* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void InstanceScene::keyDown(int key)
	{

	}

	void InstanceScene::CreateEntities()
	{
		Entity* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional.Colour = glm::vec3(6.0f, 6.0f, 8.0f);
		directional.Specular = glm::vec3(0.0f);
		directional.ShadowProjectionSize = 70.0f;
		directional.Far = 150.0f;
		ecs.AddComponent(dirLight->ID(), directional);

		Material* textured = new Material();
		textured->baseColourMaps.push_back(resources->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, false));
		textured->specularMaps.push_back(resources->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		textured->normalMaps.push_back(resources->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		textured->shininess = 5.0f;
		resources->AddMaterial("textured", textured);

		PBRMaterial* bricks = new PBRMaterial();
		bricks->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
		bricks->normalMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
		bricks->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
		bricks->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
		bricks->aoMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
		bricks->heightMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
		bricks->height_scale = -0.1;
		resources->AddMaterial("bricks", bricks);

		PBRMaterial* gold = new PBRMaterial();
		gold->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(resources->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(resources->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));
		resources->AddMaterial("gold", gold);

		Entity* baseInstance = ecs.New("Base Instance");
		ecs.AddComponent(baseInstance->ID(), ComponentGeometry(MODEL_CUBE, true, true));
		ecs.GetComponent<ComponentGeometry>(baseInstance->ID())->ApplyMaterialToModel(gold);
		const unsigned int baseInstanceID = baseInstance->ID();

		Entity* pointLight = ecs.New("Point Light");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(pointLight->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.0f, -2.0f));
		ComponentLight point = ComponentLight(POINT);
		point.Colour = glm::vec3(0.8f, 0.15f, 0.25f);
		ecs.AddComponent(pointLight->ID(), point);

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
					Entity* box = ecs.New(name);
					transform = ecs.GetComponent<ComponentTransform>(box->ID());
					transform->SetPosition(glm::vec3(originX + (j * xDistance), originY + (i * yDistance), originZ + (k * zDistance)));
					//ecs.GetComponent<ComponentGeometry>(baseInstanceID)->AddNewInstanceSource(box);
					count++;
					std::cout << "box " << count << " created" << std::endl;
				}
			}
		}

		Entity* baseInstance2 = ecs.New("Base Instance 2");
		transform = ecs.GetComponent<ComponentTransform>(baseInstance2->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.0f, 2.5f));
		ecs.AddComponent(baseInstance2->ID(), ComponentGeometry(MODEL_CUBE, true, true));
		ecs.GetComponent<ComponentGeometry>(baseInstance2->ID())->ApplyMaterialToModel(bricks);
		const unsigned int baseInstance2ID = baseInstance2->ID();

		for (int i = 0; i < yNum; i++) {
			for (int j = 0; j < xNum; j++) {
				for (int k = 0; k < zNum; k++) {
					std::string name = std::string("Box ") + std::string(std::to_string(count));
					Entity* box = ecs.New(name);
					transform = ecs.GetComponent<ComponentTransform>(box->ID());
					transform->SetPosition(glm::vec3((originX + (j * xDistance), originY + (i * yDistance), -originZ + (k * -zDistance))));
					//ecs.GetComponent<ComponentGeometry>(baseInstance2ID)->AddNewInstanceSource(box);
					count++;
					std::cout << "box " << count << " created" << std::endl;
				}
			}
		}
		std::cout << count << " box instances created" << std::endl;

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
		Entity* uiCanvas = ecs.New("Canvas");
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

	void InstanceScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}