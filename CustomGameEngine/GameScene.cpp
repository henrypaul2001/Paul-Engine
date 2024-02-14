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
		Entity* newEntity = new Entity("TestEntity");
		newEntity->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		newEntity->AddComponent(new ComponentVelocity(0.25f, 0.0f, 0.0f));
		newEntity->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));

		entityManager->AddEntity(newEntity);
	}

	void GameScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemRender(), RENDER_SYSTEMS);
	}

	void GameScene::Update()
	{
		systemManager->ActionUpdateSystems(entityManager);
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