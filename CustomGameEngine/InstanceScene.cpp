#include "InstanceScene.h"
#include "GameInputManager.h"
#include "SystemInstanceGeometryUpdate.h"
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
		directional->ShadowProjectionSize = 20.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		int xNum = 20;
		int yNum = 20;

		float originX = float(-xNum) / 2.0f;
		float originY = float(-yNum) / 2.0f;
		float originZ = -10.0f;

		float xDistance = 5.0f;
		float yDistance = 5.0f;

		int count = 0;
		for (int i = 0; i < yNum; i++) {
			for (int j = 0; j < xNum; j++) {
				std::string name = std::string("Box ") + std::string(std::to_string(count));
				//Entity* box = new Entity(name);
				//box->AddComponent(new ComponentTransform(originX + (j * xDistance), originY + (i * yDistance), originZ));
				//box->AddComponent(new ComponentGeometry(MODEL_CUBE));
				//box->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
				//box->GetBoxCollisionComponent()->CheckBroadPhaseFirst(true);
				//entityManager->AddEntity(box);
				count++;
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
		systemManager->AddSystem(new SystemInstanceGeometryUpdate(), UPDATE_SYSTEMS);
	}
}