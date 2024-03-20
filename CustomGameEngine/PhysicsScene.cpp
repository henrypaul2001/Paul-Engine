#include "PhysicsScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
#include "SystemCollisionAABB.h"
#include "SystemCollisionSphere.h"
#include "SystemCollisionSphereAABB.h"
#include "ComponentCollisionBox.h"
#include "SystemCollisionBox.h"
#include "SystemCollisionBoxAABB.h"
#include "SystemCollisionSphereBox.h"
namespace Engine {
	PhysicsScene::PhysicsScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SSAO = true;
		SetupScene();
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
		RenderManager::GetInstance()->bloomThreshold = 10.0f;
	}

	PhysicsScene::~PhysicsScene()
	{

	}

	void PhysicsScene::ChangePostProcessEffect()
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

	void PhysicsScene::ToggleSSAO()
	{
		SSAO = !SSAO;
		std::cout << "SSAO: " << SSAO << std::endl;
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
	}

	void PhysicsScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);

		float time = (float)glfwGetTime();
	}

	void PhysicsScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void PhysicsScene::Close()
	{
	}

	void PhysicsScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void PhysicsScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		else if (key == GLFW_KEY_P) {
			ToggleSSAO();
		}
		else if (key == GLFW_KEY_KP_8) {
			Entity* cube = entityManager->FindEntity("Physics Cube");
			dynamic_cast<ComponentPhysics*>(cube->GetComponent(COMPONENT_PHYSICS))->AddForce(glm::vec3(0.0f, 100.0f, 0.0f));
		}
	}

	void PhysicsScene::keyDown(int key)
	{

	}

	void PhysicsScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(1.0f);
		directional->Specular = glm::vec3(0.0f);
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.1f, -30.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 0.1f));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		entityManager->AddEntity(floor);

		Entity* physicsCube = new Entity("Physics Cube");
		physicsCube->AddComponent(new ComponentTransform(0.0f, 20.0f, -30.0f));
		physicsCube->AddComponent(new ComponentGeometry(MODEL_CUBE));
		physicsCube->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		physicsCube->AddComponent(new ComponentPhysics(10.0f));
		entityManager->AddEntity(physicsCube);
	}

	void PhysicsScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionAABB(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphere(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereAABB(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBox(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBoxAABB(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereBox(entityManager), UPDATE_SYSTEMS);

		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
	}
}