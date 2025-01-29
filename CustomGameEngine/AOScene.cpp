#include "AOScene.h"
#include "GameInputManager.h"
#include "UIText.h"
namespace Engine {
	AOScene::AOScene(SceneManager* sceneManager) : Scene(sceneManager, "AOScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(1.0f);
	}

	AOScene::~AOScene()
	{

	}

	void AOScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void AOScene::keyUp(int key)
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

	void AOScene::keyDown(int key)
	{

	}

	void AOScene::CreateEntities()
	{
		Material* bloomTest = new Material();
		bloomTest->baseColour = glm::vec3(10.0f, 10.0f, 10.0f);
		bloomTest->specular = glm::vec3(10.0f, 10.0f, 10.0f);
		bloomTest->shininess = 10.0f;
		resources->AddMaterial("Bloom Test", bloomTest);

		Material* lightCubeMaterial = new Material();
		lightCubeMaterial->baseColour = glm::vec3(8.0f, 0.0f, 15.0f);
		//lightCubeMaterial->diffuse = glm::vec3(1.0f, 0.0f, 2.0f);
		lightCubeMaterial->specular = lightCubeMaterial->baseColour;
		lightCubeMaterial->shininess = 100.0f;
		resources->AddMaterial("Light Cube Material", lightCubeMaterial);

		EntityNew* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = false;
		directional.Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional.Colour = glm::vec3(0.0f);
		directional.Specular = glm::vec3(0.0f);
		ecs.AddComponent(dirLight->ID(), directional);

		EntityNew* floor = ecs.New("Floor");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(0.0f, -1.0f, 0.0));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_PLANE));
		transform->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);

		EntityNew* wall1 = ecs.New("Wall 1");
		transform = ecs.GetComponent<ComponentTransform>(wall1->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
		ecs.AddComponent(wall1->ID(), ComponentGeometry(MODEL_PLANE));
		transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		transform->SetRotation(glm::vec3(0.0, 1.0, 0.0), 180.0f);

		EntityNew* wall2 = ecs.New("Wall 2");
		transform = ecs.GetComponent<ComponentTransform>(wall2->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.0f, -10.0f));
		ecs.AddComponent(wall2->ID(), ComponentGeometry(MODEL_PLANE));
		transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));

		EntityNew* wall3 = ecs.New("Wall 3");
		transform = ecs.GetComponent<ComponentTransform>(wall3->ID());
		transform->SetPosition(glm::vec3(10.0f, 0.0f, 0.0f));
		ecs.AddComponent(wall3->ID(), ComponentGeometry(MODEL_PLANE));
		transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		transform->SetRotation(glm::vec3(0.0, 1.0, 0.0), -90.0f);

		EntityNew* wall4 = ecs.New("Wall 4");
		transform = ecs.GetComponent<ComponentTransform>(wall4->ID());
		transform->SetPosition(glm::vec3(-10.0f, 0.0f, 0.0f));
		ecs.AddComponent(wall4->ID(), ComponentGeometry(MODEL_PLANE));
		transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		transform->SetRotation(glm::vec3(0.0, 1.0, 0.0), 90.0f);

		EntityNew* roof = ecs.New("Roof");
		transform = ecs.GetComponent<ComponentTransform>(roof->ID());
		transform->SetPosition(glm::vec3(0.0f, 5.0f, 0.0));
		ecs.AddComponent(roof->ID(), ComponentGeometry(MODEL_PLANE));
		transform->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), 90.0f);

		EntityNew* pointLight = ecs.New("Point Light");
		transform = ecs.GetComponent<ComponentTransform>(pointLight->ID());
		transform->SetPosition(glm::vec3(8.5f, 4.0f, -8.5f));
		ecs.AddComponent(pointLight->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.GetComponent<ComponentGeometry>(pointLight->ID())->ApplyMaterialToModel(lightCubeMaterial);
		ecs.GetComponent<ComponentGeometry>(pointLight->ID())->CastShadows(false);
		transform->SetScale(glm::vec3(0.25f));
		ComponentLight light = ComponentLight(POINT);
		//light.Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light.Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light.Specular = light.Colour;
		light.Ambient = light.Colour * 0.3f;
		light.CastShadows = false;
		ecs.AddComponent(pointLight->ID(), light);

		EntityNew* pointLight2 = ecs.New("Point Light2");
		transform = ecs.GetComponent<ComponentTransform>(pointLight2->ID());
		transform->SetPosition(glm::vec3(-8.5f, 4.0f, 8.5f));
		ecs.AddComponent(pointLight2->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.GetComponent<ComponentGeometry>(pointLight2->ID())->ApplyMaterialToModel(lightCubeMaterial);
		ecs.GetComponent<ComponentGeometry>(pointLight2->ID())->CastShadows(false);
		transform->SetScale(glm::vec3(0.25f));
		ComponentLight light2 = ComponentLight(POINT);
		//light2.Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light2.Colour = glm::vec3(1.0f, 0.0f, 4.0f);
		light2.Specular = light2.Colour;
		light2.Ambient = light2.Colour * 0.3f;
		light2.CastShadows = false;
		ecs.AddComponent(pointLight2->ID(), light2);

		EntityNew* pointLight3 = ecs.New("Point Light3");
		transform = ecs.GetComponent<ComponentTransform>(pointLight3->ID());
		transform->SetPosition(glm::vec3(8.5f, 4.0f, 8.5f));
		ecs.AddComponent(pointLight3->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.GetComponent<ComponentGeometry>(pointLight3->ID())->ApplyMaterialToModel(lightCubeMaterial);
		ecs.GetComponent<ComponentGeometry>(pointLight3->ID())->CastShadows(false);
		transform->SetScale(glm::vec3(0.25f));
		ComponentLight light3 = ComponentLight(POINT);
		//light3.Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light3.Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light3.Specular = light3.Colour;
		light3.Ambient = light3.Colour * 0.3f;
		light3.CastShadows = false;
		ecs.AddComponent(pointLight3->ID(), light3);

		EntityNew* pointLight4 = ecs.New("Point Light4");
		transform = ecs.GetComponent<ComponentTransform>(pointLight4->ID());
		transform->SetPosition(glm::vec3(-8.5f, 4.0f, -8.5f));
		ecs.AddComponent(pointLight4->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.GetComponent<ComponentGeometry>(pointLight4->ID())->ApplyMaterialToModel(lightCubeMaterial);
		ecs.GetComponent<ComponentGeometry>(pointLight4->ID())->CastShadows(false);
		transform->SetScale(glm::vec3(0.25f));
		ComponentLight light4 = ComponentLight(POINT);
		//light4.Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light4.Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light4.Specular = light4.Colour;
		light4.Ambient = light4.Colour * 0.3f;
		light4.CastShadows = false;
		ecs.AddComponent(pointLight4->ID(), light4);

		EntityNew* pointLight5 = ecs.New("Point Light5");
		transform = ecs.GetComponent<ComponentTransform>(pointLight5->ID());
		transform->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
		ecs.AddComponent(pointLight5->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.GetComponent<ComponentGeometry>(pointLight5->ID())->ApplyMaterialToModel(lightCubeMaterial);
		ecs.GetComponent<ComponentGeometry>(pointLight5->ID())->CastShadows(false);
		transform->SetScale(glm::vec3(0.25f));
		ComponentLight light5 = ComponentLight(POINT);
		//light5.Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light5.Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light5.Specular = light5.Colour;
		light5.Ambient = light5.Colour * 0.0f;
		light5.Linear = 0.027f;
		light5.Quadratic = 0.0028f;
		light5.CastShadows = false;
		ecs.AddComponent(pointLight5->ID(), light5);

		EntityNew* backpack = ecs.New("Backpack");
		transform = ecs.GetComponent<ComponentTransform>(backpack->ID());
		transform->SetPosition(glm::vec3(0.0f, 1.05f, -1.0f));
		stbi_set_flip_vertically_on_load(true);
		ecs.AddComponent(backpack->ID(), ComponentGeometry("Models/backpack/backpack.obj", false));
		stbi_set_flip_vertically_on_load(false);
		transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), -20.0f);
		//ecs.GetComponent<ComponentGeometry>(backpack->ID())->GetModel()->ApplyMaterialToAllMesh(bloomTest);

#pragma region UI
		TextFont* font = resources->LoadTextFont("Fonts/arial.ttf");
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

	void AOScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}

	void AOScene::ChangePostProcessEffect()
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

	void AOScene::Update()
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

	void AOScene::Render()
	{
		Scene::Render();
	}

	void AOScene::Close()
	{
		std::cout << "Closing AO scene" << std::endl;
	}
}