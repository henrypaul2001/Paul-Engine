#include "GameScene.h"
#include <iostream>
#include <glm/vec3.hpp>
#include "SystemPhysics.h"
#include <glad/glad.h>
#include "EntityManager.h"
#include "SystemManager.h"
#include "GameInputManager.h"
namespace Engine
{
	GameScene::GameScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		entityManager = new EntityManager();
		systemManager = new SystemManager();
		inputManager = new GameInputManager();

		SetupScene();
	}

	GameScene::~GameScene()
	{
		delete entityManager;
		delete systemManager;
		delete inputManager;
	}

	void GameScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);

		// Set-up framebuffers

		// Compile shaders

		// Configure shaders

		// Prepare scene

		CreateSystems();
		CreateEntities();
	}

	void GameScene::CreateEntities()
	{
		Material* newMeshMaterial = new Material();
		newMeshMaterial->diffuse = glm::vec3(0.8f, 0.0f, 0.8f);
		newMeshMaterial->specular = glm::vec3(1.0f, 0.0f, 1.0f);
		newMeshMaterial->shininess = 100.0f;

		Material* cobbleFloor = new Material();
		cobbleFloor->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE));
		cobbleFloor->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL));
		cobbleFloor->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR));
		cobbleFloor->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/displace.png", TEXTURE_DISPLACE));
		cobbleFloor->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		cobbleFloor->specular = glm::vec3(0.5f, 0.5f, 0.5f);
		cobbleFloor->shininess = 60.0f;
		cobbleFloor->height_scale = -0.05f;

		Material* brickWall = new Material();
		brickWall->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/diffuse.jpg", TEXTURE_DIFFUSE));
		brickWall->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/normal.jpg", TEXTURE_NORMAL));
		//brickWall->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/specular.jpg", TEXTURE_SPECULAR));
		brickWall->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/displace.jpg", TEXTURE_DISPLACE));
		brickWall->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		brickWall->specular = glm::vec3(0.5f, 0.5f, 0.5f);
		brickWall->shininess = 60.0f;
		brickWall->height_scale = 0.1f;

		Entity* brickPlane = new Entity("Brick Wall");
		ComponentTransform* bricktransform = new ComponentTransform(0.0f, -5.0f, 0.0f);
		bricktransform->SetScale(glm::vec3(5.0f, 5.0f, 1.0f));
		bricktransform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		brickPlane->AddComponent(bricktransform);
		brickPlane->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(brickPlane->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(brickWall);
		entityManager->AddEntity(brickPlane);

		Entity* defaultCube = new Entity("Default Cube");
		defaultCube->AddComponent(new ComponentTransform(3.0f, -1.5f, 0.0f));
		dynamic_cast<ComponentTransform*>(defaultCube->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(2.5f));
		defaultCube->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(defaultCube->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(brickWall);
		entityManager->AddEntity(defaultCube);

		Entity* defaultPlane = new Entity("Default Plane");
		ComponentTransform* transform = new ComponentTransform(-6.0f, -5.0f, 0.0f);
		transform->SetScale(glm::vec3(5.0f, 5.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		defaultPlane->AddComponent(transform);
		defaultPlane->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(defaultPlane->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(cobbleFloor);
		dynamic_cast<ComponentGeometry*>(defaultPlane->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(1.0f);
		entityManager->AddEntity(defaultPlane);

		Entity* backpack = new Entity("Backpack");
		backpack->AddComponent(new ComponentTransform(0.0f, 2.0f, -5.0f));
		stbi_set_flip_vertically_on_load(true);
		backpack->AddComponent(new ComponentGeometry("Models/backpack/backpack.obj", false));
		stbi_set_flip_vertically_on_load(false);
		//dynamic_cast<ComponentGeometry*>(backpack->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(ResourceManager::GetInstance()->DefaultMaterial());
		entityManager->AddEntity(backpack);

		Entity* rock = new Entity("Rock");
		rock->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		rock->AddComponent(new ComponentVelocity(0.5f, 0.0f, 0.0f));
		rock->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		ComponentLight* rockLight = new ComponentLight(POINT);
		rockLight->Colour = glm::vec3(0.8f, 0.0f, 0.0f);
		rockLight->Specular = glm::vec3(0.8f, 0.0f, 0.0f);
		rockLight->Ambient = glm::vec3(0.2f, 0.0f, 0.0f);
		rock->AddComponent(rockLight);
		entityManager->AddEntity(rock);

		Entity* rockChild = new Entity("RockChild");
		rockChild->AddComponent(new ComponentTransform(0.0f, 3.5f, 0.0f));
		rockChild->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		dynamic_cast<ComponentTransform*>(rockChild->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
		dynamic_cast<ComponentTransform*>(rockChild->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), 180.0f);
		dynamic_cast<ComponentTransform*>(rockChild->GetComponent(COMPONENT_TRANSFORM))->SetParent(rock);
		entityManager->AddEntity(rockChild);

		Entity* rockChild2 = new Entity("RockChild2");
		rockChild2->AddComponent(new ComponentTransform(0.0f, -10.0f, 0.0f));
		rockChild2->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		dynamic_cast<ComponentTransform*>(rockChild2->GetComponent(COMPONENT_TRANSFORM))->SetParent(rockChild);
		entityManager->AddEntity(rockChild2);

		Entity* rockChild3 = new Entity("RockChild3");
		rockChild3->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		rockChild3->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		dynamic_cast<ComponentTransform*>(rockChild3->GetComponent(COMPONENT_TRANSFORM))->SetParent(rockChild2);
		entityManager->AddEntity(rockChild3);

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		dirLight->AddComponent(new ComponentLight(DIRECTIONAL));
		entityManager->AddEntity(dirLight);

		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(0.0f, 2.0f, -7.5f));
		spotLight->AddComponent(new ComponentLight(SPOT));
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(0.0f, 0.0f, 1.0f);
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(backpack);
		entityManager->AddEntity(spotLight);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(0.0f, -2.0f, 2.0f));
		dynamic_cast<ComponentTransform*>(pointLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		pointLight->AddComponent(new ComponentLight(POINT));
		pointLight->AddComponent(new ComponentGeometry(MODEL_CUBE));
		pointLight->AddComponent(new ComponentVelocity(glm::vec3(1.0f, 0.0f, 0.0f)));
		//dynamic_cast<ComponentLight*>(pointLight->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(0.0f, 0.0f, 1.0f);
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(backpack);
		entityManager->AddEntity(pointLight);

		Entity* testSphere = new Entity("Test Sphere");
		testSphere->AddComponent(new ComponentTransform(-2.0f, 0.0f, 3.0f));
		testSphere->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		entityManager->AddEntity(testSphere);
	}

	void GameScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemRender(), RENDER_SYSTEMS);
	}

	void GameScene::Update()
	{
		systemManager->ActionUpdateSystems(entityManager);
		float time = (float)glfwGetTime();
		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Rock")->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(sin(time) * 1.0f, sin(time) * 1.0f, sin(time) * 1.0f));
		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 0.0f, 1.0f), sin(time) * 180.0f);
		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild2")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), sin(time) * 180.0f);
		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild3")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0f, 1.0f, 1.0f), sin(time) * 180.0f);

		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Backpack")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 45.0f);

		dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Point Light")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(1.0f, 0.0f, 0.0f) * sin(time) * 10.0f);
	}

	void GameScene::Render()
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render entities
		systemManager->ActionRenderSystems(entityManager);
	}

	void GameScene::Close()
	{
		std::cout << "Closing game scene" << std::endl;
	}
}