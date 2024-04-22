#include "InstanceScene.h"
#include "GameInputManager.h"
namespace Engine {
	InstanceScene::InstanceScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		RenderManager::GetInstance()->bloomThreshold = 10.0f;
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
		directional->Colour = glm::vec3(1.0f);
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

		Entity* baseInstance = new Entity("Base Instance");
		baseInstance->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		baseInstance->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		baseInstance->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(textured);
		entityManager->AddEntity(baseInstance);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(0.0f, 0.0f, -2.0f));
		pointLight->AddComponent(new ComponentLight(POINT));
		pointLight->GetLightComponent()->Colour = glm::vec3(0.8f, 0.15f, 0.25f);
		entityManager->AddEntity(pointLight);

		int xNum = 10;
		int yNum = 10;
		int zNum = 10;

		float originX = float(-xNum) / 2.0f;
		float originY = float(-yNum) / 2.0f;
		float originZ = -10.0f;

		float xDistance = 2.5f;
		float yDistance = 2.5f;
		float zDistance = -2.5f;

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
		std::cout << count << " box instances created" << std::endl;
	}

	void InstanceScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
	}
}