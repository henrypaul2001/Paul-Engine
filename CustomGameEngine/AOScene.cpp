#include "AOScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
namespace Engine {
	AOScene::AOScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		RenderManager::GetInstance()->bloomThreshold = 1.0f;
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
	}

	void AOScene::keyDown(int key)
	{

	}

	void AOScene::CreateEntities()
	{
		Material* bloomTest = new Material();
		bloomTest->diffuse = glm::vec3(10.0f, 10.0f, 10.0f);
		bloomTest->specular = glm::vec3(10.0f, 10.0f, 10.0f);
		bloomTest->shininess = 10.0f;
		bloomTest->isTransparent = false;

		Material* lightCubeMaterial = new Material();
		lightCubeMaterial->diffuse = glm::vec3(8.0f, 0.0f, 15.0f);
		//lightCubeMaterial->diffuse = glm::vec3(1.0f, 0.0f, 2.0f);
		lightCubeMaterial->specular = lightCubeMaterial->diffuse;
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
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(lightCubeMaterial);
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
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(lightCubeMaterial);
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
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(lightCubeMaterial);
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
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(lightCubeMaterial);
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
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(lightCubeMaterial);
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
	}

	void AOScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
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