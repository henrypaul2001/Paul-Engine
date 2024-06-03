#include "GameSceneManager.h"
#include "GameScene.h"
#include "SponzaScene.h"
#include "AOScene.h"
#include "CollisionScene.h"
#include "PBRScene.h"
#include "PhysicsScene.h"
#include "InstanceScene.h"
#include "MainMenu.h"
#include "AnimationScene.h"
#include "AudioScene.h"
#include <iostream>
namespace Engine
{
	GameSceneManager::GameSceneManager(int width, int height, int windowXPos, int windowYPos) : SceneManager(width, height, windowXPos, windowYPos)
	{

	}

	GameSceneManager::~GameSceneManager()
	{

	}

	void GameSceneManager::ChangeScene(SceneTypes sceneType)
	{
		std::cout << "Checking if current scene exists" << std::endl;
		if (this->scene != nullptr) { 
			scene->Close(); 
			delete scene;
		}

		std::cout << "Attempting to create scene '" << sceneType << "'" << std::endl;
		Scene* newScene{};
		switch (sceneType) {
		case SCENE_MAIN_MENU:
			newScene = new MainMenu(this);
			break;
		case SCENE_GAME:
			newScene = new GameScene(this);
			break;
		case SCENE_GAME_OVER:
			break;
		case SCENE_WIN:
			break;
		case SCENE_SPONZA:
			newScene = new SponzaScene(this);
			break;
		case SCENE_AO:
			newScene = new AOScene(this);
			break;
		case SCENE_PBR:
			newScene = new PBRScene(this);
			break;
		case SCENE_COLLISIONS:
			newScene = new CollisionScene(this);
			break;
		case SCENE_PHYSICS:
			newScene = new PhysicsScene(this);
			break;
		case SCENE_INSTANCED:
			newScene = new InstanceScene(this);
			break;
		case SCENE_ANIMATION:
			newScene = new AnimationScene(this);
			break;
		case SCENE_AUDIO:
			newScene = new AudioScene(this);
			break;
		case SCENE_NONE:
			newScene = nullptr;
			break;
		}
		std::cout << "Scene '" << sceneType << "' created" << std::endl;
		scene = newScene;
	}

	void GameSceneManager::StartMenu()
	{

	}

	void GameSceneManager::StartNewGame()
	{
		ChangeScene(SCENE_AUDIO);
	}
}