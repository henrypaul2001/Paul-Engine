#include "AOScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
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
			Entity* canvas = entityManager->FindEntity("Canvas");

			canvas->GetUICanvasComponent()->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[9]->SetActive(!renderGeometryColliders);
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

		Material* lightCubeMaterial = new Material();
		lightCubeMaterial->baseColour = glm::vec3(8.0f, 0.0f, 15.0f);
		//lightCubeMaterial->diffuse = glm::vec3(1.0f, 0.0f, 2.0f);
		lightCubeMaterial->specular = lightCubeMaterial->baseColour;
		lightCubeMaterial->shininess = 100.0f;

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = false;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(0.0f);
		directional->Specular = glm::vec3(0.0f);
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.0f, 0.0));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(floor);

		Entity* wall1 = new Entity("Wall 1");
		wall1->AddComponent(new ComponentTransform(0.0f, 0.0f, 10.0f));
		wall1->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 180.0f);
		entityManager->AddEntity(wall1);

		Entity* wall2 = new Entity("Wall 2");
		wall2->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		wall2->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(wall2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		entityManager->AddEntity(wall2);

		Entity* wall3 = new Entity("Wall 3");
		wall3->AddComponent(new ComponentTransform(10.0f, 0.0f, 0.0f));
		wall3->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), -90.0f);
		entityManager->AddEntity(wall3);

		Entity* wall4 = new Entity("Wall 4");
		wall4->AddComponent(new ComponentTransform(-10.0f, 0.0f, 0.0f));
		wall4->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 90.0f);
		entityManager->AddEntity(wall4);

		Entity* roof = new Entity("Roof");
		roof->AddComponent(new ComponentTransform(0.0f, 5.0f, 0.0));
		roof->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(roof->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(roof->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), 90.0f);
		entityManager->AddEntity(roof);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(8.5f, 4.0f, -8.5f));
		pointLight->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(lightCubeMaterial);
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light = new ComponentLight(POINT);
		//light->Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light->Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light->Specular = light->Colour;
		light->Ambient = light->Colour * 0.3f;
		light->CastShadows = false;
		pointLight->AddComponent(light);
		entityManager->AddEntity(pointLight);

		Entity* pointLight2 = new Entity("Point Light2");
		pointLight2->AddComponent(new ComponentTransform(-8.5f, 4.0f, 8.5f));
		pointLight2->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(lightCubeMaterial);
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light2 = new ComponentLight(POINT);
		//light2->Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light2->Colour = glm::vec3(1.0f, 0.0f, 4.0f);
		light2->Specular = light2->Colour;
		light2->Ambient = light2->Colour * 0.3f;
		light2->CastShadows = false;
		pointLight2->AddComponent(light2);
		entityManager->AddEntity(pointLight2);

		Entity* pointLight3 = new Entity("Point Light3");
		pointLight3->AddComponent(new ComponentTransform(8.5f, 4.0f, 8.5f));
		pointLight3->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(lightCubeMaterial);
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light3 = new ComponentLight(POINT);
		//light3->Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light3->Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light3->Specular = light3->Colour;
		light3->Ambient = light3->Colour * 0.3f;
		light3->CastShadows = false;
		pointLight3->AddComponent(light3);
		entityManager->AddEntity(pointLight3);

		Entity* pointLight4 = new Entity("Point Light4");
		pointLight4->AddComponent(new ComponentTransform(-8.5f, 4.0f, -8.5f));
		pointLight4->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(lightCubeMaterial);
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light4 = new ComponentLight(POINT);
		//light4->Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light4->Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light4->Specular = light4->Colour;
		light4->Ambient = light4->Colour * 0.3f;
		light4->CastShadows = false;
		pointLight4->AddComponent(light4);
		entityManager->AddEntity(pointLight4);

		Entity* pointLight5 = new Entity("Point Light5");
		pointLight5->AddComponent(new ComponentTransform(0.0f, 2.0f, 0.0f));
		pointLight5->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(lightCubeMaterial);
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight5->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light5 = new ComponentLight(POINT);
		//light5->Colour = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		light5->Colour = glm::vec3(0.5f, 0.0f, 2.0f);
		light5->Specular = light5->Colour;
		light5->Ambient = light5->Colour * 0.0f;
		light5->Linear = 0.027f;
		light5->Quadratic = 0.0028f;
		light5->CastShadows = false;
		//pointLight5->AddComponent(light5);
		entityManager->AddEntity(pointLight5);

		Entity* backpack = new Entity("Backpack");
		backpack->AddComponent(new ComponentTransform(0.0f, 1.05f, -1.0f));
		stbi_set_flip_vertically_on_load(true);
		backpack->AddComponent(new ComponentGeometry("Models/backpack/backpack.obj", false));
		stbi_set_flip_vertically_on_load(false);
		dynamic_cast<ComponentTransform*>(backpack->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -20.0f);
		//dynamic_cast<ComponentGeometry*>(backpack->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bloomTest);
		entityManager->AddEntity(backpack);

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

	void AOScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
	}

	void AOScene::ChangePostProcessEffect()
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

	void AOScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);

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

	void AOScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void AOScene::Close()
	{
		std::cout << "Closing AO scene" << std::endl;
	}
}