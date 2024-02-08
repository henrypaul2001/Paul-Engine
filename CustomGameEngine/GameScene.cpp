#include "GameScene.h"
#include <iostream>
#include <glm/vec3.hpp>
#include "ComponentTransform.h"
#include "ComponentVelocity.h"
#include "SystemPhysics.h"
namespace Engine
{
	GameScene::GameScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		entityManager = EntityManager();
		systemManager = SystemManager();

		SetupScene();
	}

	GameScene::~GameScene()
	{
		delete& entityManager;
		delete& systemManager;
	}

	void GameScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);

		// Set-up framebuffers

		// Compile shaders

		// Configure shaders

		// Prepare scene

		CreateEntities();
		CreateSystems();
	}

	void GameScene::CreateEntities()
	{
		Entity* newEntity = new Entity("TestEntity");
		newEntity->AddComponent(new ComponentTransform(10.0, 0, 10.0));
		newEntity->AddComponent(new ComponentVelocity(1.0, 0, 0));

		entityManager.AddEntity(newEntity);
	}

	void GameScene::CreateSystems()
	{
		systemManager.AddSystem(new SystemPhysics());
	}

	void GameScene::Update()
	{
		systemManager.ActionSystems(&entityManager);
	}

	void GameScene::Render()
	{
	}

	void GameScene::Close()
	{
		std::cout << "Closing game scene" << std::endl;
	}
}