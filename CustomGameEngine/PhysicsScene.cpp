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
		ballCount = 0;
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

		Entity* ball = entityManager->FindEntity("Physics Ball");
		//std::cout << "Velocity y = " << dynamic_cast<ComponentPhysics*>(ball->GetComponent(COMPONENT_PHYSICS))->Velocity().y << std::endl;
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
			std::string name = std::string("Ball ") + std::string(std::to_string(ballCount));
			std::cout << "Creating " << name << std::endl;
			Entity* newBall = new Entity(name);
			newBall->AddComponent(new ComponentTransform(6.5f, 8.5f, -20.0f));
			newBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
			newBall->AddComponent(new ComponentCollisionSphere(1.0f, true));
			newBall->AddComponent(new ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5
			dynamic_cast<ComponentCollisionSphere*>(newBall->GetComponent(COMPONENT_COLLISION_SPHERE))->IsMovedByCollisions(true);
			entityManager->AddEntity(newBall);
			std::cout << "Created " << name << std::endl;
			ballCount++;
		}
		else if (key == GLFW_KEY_G) {
			SystemPhysics* physics = dynamic_cast<SystemPhysics*>(systemManager->FindSystem(SYSTEM_PHYSICS, UPDATE_SYSTEMS));
			glm::vec3 axis = physics->GravityAxis();

			physics->Gravity(-axis);
		}
	}

	void PhysicsScene::keyDown(int key)
	{

	}

	void PhysicsScene::CreateEntities()
	{
		Material* textured = new Material();
		textured->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, false));
		textured->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		textured->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		textured->shininess = 5.0f;

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(1.0f);
		directional->Specular = glm::vec3(0.0f);
		directional->Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* leftWall = new Entity("Left Wall");
		leftWall->AddComponent(new ComponentTransform(-6.15f, -20.0f, -20.0f));
		leftWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(leftWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(leftWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(-45.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		leftWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		dynamic_cast<ComponentCollisionBox*>(leftWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(leftWall);

		Entity* rightWall = new Entity("Right Wall");
		rightWall->AddComponent(new ComponentTransform(6.15f, -20.0f, -20.0f));
		rightWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(rightWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(rightWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(45.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		rightWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		dynamic_cast<ComponentCollisionBox*>(rightWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(rightWall);

		Entity* backWall = new Entity("Back Wall");
		backWall->AddComponent(new ComponentTransform(0.0f, -20.0f, -31.5f));
		backWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(backWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(backWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(glm::radians(88.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		backWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		dynamic_cast<ComponentCollisionBox*>(backWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(backWall);

		Entity* frontWall = new Entity("Front Wall");
		frontWall->AddComponent(new ComponentTransform(0.0f, -20.0f, -8.5f));
		frontWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(frontWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(frontWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(glm::radians(-88.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		frontWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		dynamic_cast<ComponentCollisionBox*>(frontWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(frontWall);

		Entity* physicsBall = new Entity("Physics Ball");
		physicsBall->AddComponent(new ComponentTransform(8.0f, 30.0f, -20.0f));
		physicsBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		physicsBall->AddComponent(new ComponentCollisionSphere(1.0f, true));
		physicsBall->AddComponent(new ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5
		entityManager->AddEntity(physicsBall);

		Entity* physicsBall2 = new Entity("Physics Ball 2");
		physicsBall2->AddComponent(new ComponentTransform(6.5f, 8.5f, -20.0f));
		physicsBall2->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		physicsBall2->AddComponent(new ComponentCollisionSphere(1.0f, true));
		physicsBall2->AddComponent(new ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5
		dynamic_cast<ComponentCollisionSphere*>(physicsBall2->GetComponent(COMPONENT_COLLISION_SPHERE))->IsMovedByCollisions(true);
		entityManager->AddEntity(physicsBall2);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -2.0f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		floor->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		dynamic_cast<ComponentCollisionBox*>(floor->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(floor);

		Entity* inelasticBall = new Entity("Inelastic Ball");
		inelasticBall->AddComponent(new ComponentTransform(2.0f, 4.0f, -1.0f));
		inelasticBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		//dynamic_cast<ComponentGeometry*>(inelasticBall->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(textured);
		inelasticBall->AddComponent(new ComponentCollisionSphere(1.0f, true));
		inelasticBall->AddComponent(new ComponentPhysics(10.0f, 0.47f, 0.5f, 0.15f, true));
		entityManager->AddEntity(inelasticBall);

		Entity* elasticBall = new Entity("Elastic Ball");
		elasticBall->AddComponent(new ComponentTransform(2.0f, 8.0f, -1.0f));
		elasticBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentTransform*>(elasticBall->GetComponent(COMPONENT_TRANSFORM))->SetScale(0.5f);
		//dynamic_cast<ComponentGeometry*>(elasticBall->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(textured);
		elasticBall->AddComponent(new ComponentCollisionSphere(1.0f, true));
		elasticBall->AddComponent(new ComponentPhysics(10.0f, 0.47f, 0.5f, 1.0f, true));
		entityManager->AddEntity(elasticBall);

		Entity* box = new Entity("Box");
		box->AddComponent(new ComponentTransform(-5.0f, 10.0f, -1.0f));
		dynamic_cast<ComponentTransform*>(box->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.5f, 0.0f, 1.0f), 45.0f);
		box->AddComponent(new ComponentGeometry(MODEL_CUBE));
		box->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		box->AddComponent(new ComponentPhysics(5.0f, 1.05f, 2.0f, 0.5f, true, true));
		entityManager->AddEntity(box);

		//Entity* box2 = new Entity("Box 2");
		//box2->AddComponent(new ComponentTransform(-4.5f, 13.0f, -1.0f));
		//dynamic_cast<ComponentTransform*>(box2->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.5f, 0.0f, 1.0f), 45.0f);
		//box2->AddComponent(new ComponentGeometry(MODEL_CUBE));
		//box2->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		//box2->AddComponent(new ComponentPhysics(5.0f, 1.05f, 2.0f, 0.5f, true, true));
		//entityManager->AddEntity(box2);
	}

	void PhysicsScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphere(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBox(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBoxAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereBox(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddCollisionResponseSystem(new CollisionResponder(collisionManager));
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
	}
}