#include "CollisionScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
namespace Engine {
	CollisionScene::CollisionScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SSAO = true;
		SetupScene();
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
		RenderManager::GetInstance()->bloomThreshold = 1.0f;
	}

	CollisionScene::~CollisionScene()
	{

	}

	void CollisionScene::ChangePostProcessEffect()
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

	void CollisionScene::ToggleSSAO()
	{
		SSAO = !SSAO;
		std::cout << "SSAO: " << SSAO << std::endl;
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
	}

	void CollisionScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);
	}

	void CollisionScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void CollisionScene::Close()
	{
	}

	void CollisionScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void CollisionScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		else if (key == GLFW_KEY_P) {
			ToggleSSAO();
		}
	}

	void CollisionScene::keyDown(int key)
	{

	}

	void CollisionScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = false;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(1.0f);
		directional->Specular = glm::vec3(0.0f);
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.0f, 0.0));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(floor);
	}

	void CollisionScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
	}
}