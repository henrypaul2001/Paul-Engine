#include "CollisionScene.h"
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

		float time = (float)glfwGetTime();

		Entity* left = entityManager->FindEntity("Collision Test Left");
		Entity* right = entityManager->FindEntity("Collision Test Right");
		if (dynamic_cast<ComponentCollision*>(left->GetComponent(COMPONENT_COLLISION_AABB))->IsCollidingWithEntity(right)) {
			dynamic_cast<ComponentVelocity*>(left->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(left->GetComponent(COMPONENT_VELOCITY))->Velocity());
		}

		Entity* wall = entityManager->FindEntity("Box Sphere Collision Test");
		if (dynamic_cast<ComponentCollision*>(left->GetComponent(COMPONENT_COLLISION_AABB))->IsCollidingWithEntity(wall)) {
			dynamic_cast<ComponentVelocity*>(left->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(left->GetComponent(COMPONENT_VELOCITY))->Velocity());
		}

		Entity* leftSphere = entityManager->FindEntity("Sphere Collision Test Left");
		Entity* rightSphere = entityManager->FindEntity("Sphere Collision Test Right");

		if (dynamic_cast<ComponentCollisionSphere*>(leftSphere->GetComponent(COMPONENT_COLLISION_SPHERE))->IsCollidingWithEntity(rightSphere)) {
			dynamic_cast<ComponentVelocity*>(leftSphere->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(leftSphere->GetComponent(COMPONENT_VELOCITY))->Velocity());
		}

		Entity* rotatingBox = entityManager->FindEntity("Rotated Box");
		Entity* rotatedBox = entityManager->FindEntity("Rotated Box 2");
		Entity* sphere = entityManager->FindEntity("Sphere vs Box");

		float rotation = dynamic_cast<ComponentTransform*>(rotatingBox->GetComponent(COMPONENT_TRANSFORM))->RotationAngle();
		dynamic_cast<ComponentTransform*>(rotatingBox->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 45.0f);

		if (dynamic_cast<ComponentCollisionBox*>(rotatingBox->GetComponent(COMPONENT_COLLISION_BOX))->IsCollidingWithEntity(rotatedBox)) {
			dynamic_cast<ComponentVelocity*>(rotatingBox->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(rotatingBox->GetComponent(COMPONENT_VELOCITY))->Velocity());
		}

		if (dynamic_cast<ComponentCollisionBox*>(rotatingBox->GetComponent(COMPONENT_COLLISION_BOX))->IsCollidingWithEntity(sphere)) {
			dynamic_cast<ComponentVelocity*>(rotatingBox->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(rotatingBox->GetComponent(COMPONENT_VELOCITY))->Velocity());
		}

		if (dynamic_cast<ComponentCollisionBox*>(rotatingBox->GetComponent(COMPONENT_COLLISION_BOX))->IsCollidingWithEntity(wall)) {
			dynamic_cast<ComponentVelocity*>(rotatingBox->GetComponent(COMPONENT_VELOCITY))->SetVelocity(-dynamic_cast<ComponentVelocity*>(rotatingBox->GetComponent(COMPONENT_VELOCITY))->Velocity());
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
		collisionTestLeft->AddComponent(new ComponentTransform(-4.0f, 0.5f, 0.0f));
		dynamic_cast<ComponentTransform*>(collisionTestLeft->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		collisionTestLeft->AddComponent(new ComponentGeometry(MODEL_CUBE));
		collisionTestLeft->AddComponent(new ComponentVelocity(glm::vec3(10.0f, 0.0f, 0.0f)));
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

		Entity* boxSphereCollisionTest = new Entity("Box Sphere Collision Test");
		boxSphereCollisionTest->AddComponent(new ComponentTransform(-7.0f, 0.25f, -2.0f));
		dynamic_cast<ComponentTransform*>(boxSphereCollisionTest->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(1.0f, 2.0f, 8.0f));
		boxSphereCollisionTest->AddComponent(new ComponentGeometry(MODEL_CUBE));
		boxSphereCollisionTest->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		entityManager->AddEntity(boxSphereCollisionTest);

		Entity* rotatedBoxCollider1 = new Entity("Rotated Box");
		rotatedBoxCollider1->AddComponent(new ComponentTransform(-4.0f, 1.5f, 3.75f));
		dynamic_cast<ComponentTransform*>(rotatedBoxCollider1->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f);
		dynamic_cast<ComponentTransform*>(rotatedBoxCollider1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		rotatedBoxCollider1->AddComponent(new ComponentGeometry(MODEL_CUBE));
		rotatedBoxCollider1->AddComponent(new ComponentCollisionBox(-1.0f, -1.0, -1.0f, 1.0f, 1.0f, 1.0f, true));
		rotatedBoxCollider1->AddComponent(new ComponentVelocity(2.0f, 0.0f, 0.0f));
		entityManager->AddEntity(rotatedBoxCollider1);

		Entity* rotatedBoxCollider2 = new Entity("Rotated Box 2");
		rotatedBoxCollider2->AddComponent(new ComponentTransform(4.0f, 0.65f, 8.0f));
		dynamic_cast<ComponentTransform*>(rotatedBoxCollider2->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f);
		dynamic_cast<ComponentTransform*>(rotatedBoxCollider2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		rotatedBoxCollider2->AddComponent(new ComponentGeometry(MODEL_CUBE));
		rotatedBoxCollider2->AddComponent(new ComponentCollisionBox(-1.0f, -1.0, -1.0f, 1.0f, 1.0f, 1.0f, true));
		entityManager->AddEntity(rotatedBoxCollider2);

		Entity* sphereBoxCollisionTest = new Entity("Sphere vs Box");
		sphereBoxCollisionTest->AddComponent(new ComponentTransform(6.0f, 1.0f, 3.75f));
		dynamic_cast<ComponentTransform*>(sphereBoxCollisionTest->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		sphereBoxCollisionTest->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		sphereBoxCollisionTest->AddComponent(new ComponentCollisionSphere(1.0f, true));
		entityManager->AddEntity(sphereBoxCollisionTest);
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
		systemManager->AddSystem(new SystemCollisionSphereAABB(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBox(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBoxAABB(entityManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereBox(entityManager), UPDATE_SYSTEMS);
	}
}