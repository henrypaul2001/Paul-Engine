#include "GameScene.h"
namespace Engine
{
	GameScene::GameScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		entityManager = EntityManager();
		systemManager = SystemManager();

		CreateEntities();
		CreateSystems();
	}

	GameScene::~GameScene()
	{
		delete& entityManager;
		delete& systemManager;
	}

	void GameScene::CreateEntities()
	{

	}

	void GameScene::CreateSystems()
	{

	}

	void GameScene::Update()
	{
	}

	void GameScene::Render()
	{
	}

	void GameScene::Close()
	{
		delete this;
	}
}