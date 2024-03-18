#include "CollisionScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
#include "SystemCollisionAABB.h"
#include "SystemCollisionSphere.h"
namespace Engine {
	CollisionScene::CollisionScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SSAO = true;
		SetupScene();
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
		RenderManager::GetInstance()->bloomThreshold = 10.0f;
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

		Entity* left = entityManager->FindEntity("Collision Test Left");
		Entity* right = entityManager->FindEntity("Collision Test Right");
		if (dynamic_cast<ComponentCollision*>(left->GetComponent(COMPONENT_COLLISION_AABB))->IsCollidingWithEntity(right)) {
			dynamic_cast<ComponentVelocity*>(left->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(left->GetComponent(COMPONENT_VELOCITY))->Velocity());
		}
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
		directional->CastShadows = true;
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

		Entity* collisionTestLeft = new Entity("Collision Test Left");
		collisionTestLeft->AddComponent(new ComponentTransform(-4.0f, 1.99f, 0.0f));
		dynamic_cast<ComponentTransform*>(collisionTestLeft->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		collisionTestLeft->AddComponent(new ComponentGeometry(MODEL_CUBE));
		collisionTestLeft->AddComponent(new ComponentVelocity(glm::vec3(1.0f, 0.0f, 0.0f)));
		collisionTestLeft->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		entityManager->AddEntity(collisionTestLeft);

		Entity* collisionTestRight = new Entity("Collision Test Right");
		collisionTestRight->AddComponent(new ComponentTransform(4.0f, 0.5f, 0.0f));
		collisionTestRight->AddComponent(new ComponentGeometry(MODEL_CUBE));
		collisionTestRight->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		entityManager->AddEntity(collisionTestRight);

		Entity* sphereCollisionTestLeft = new Entity("Sphere Collision Test Left");
		sphereCollisionTestLeft->AddComponent(new ComponentTransform(-4.0f, 1.5f, -2.0f));
		dynamic_cast<ComponentTransform*>(sphereCollisionTestLeft->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		sphereCollisionTestLeft->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		sphereCollisionTestLeft->AddComponent(new ComponentCollisionSphere(1.0f, true));
		sphereCollisionTestLeft->AddComponent(new ComponentVelocity(1.0f, 0.0f, 0.0f));
		entityManager->AddEntity(sphereCollisionTestLeft);

		Entity* sphereCollisionTestRight = new Entity("Sphere Collision Test Right");
		sphereCollisionTestRight->AddComponent(new ComponentTransform(4.0f, 0.5f, -2.0f));
		sphereCollisionTestRight->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		sphereCollisionTestRight->AddComponent(new ComponentCollisionSphere(1.0f, true));
		entityManager->AddEntity(sphereCollisionTestRight);
	}

	void CollisionScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionAABB(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphere(entityManager), UPDATE_SYSTEMS);
	}
}