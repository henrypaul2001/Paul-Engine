#include "GameSceneManager.h"
//#include "GameScene.h"
//#include "SponzaScene.h"
//#include "AOScene.h"
//#include "CollisionScene.h"
//#include "PBRScene.h"
//#include "PhysicsScene.h"
//#include "InstanceScene.h"
//#include "MainMenu.h"
//#include "AnimationScene.h"
//#include "AudioScene.h"
#include "ParticleScene.h"
#include "AIScene.h"
#include "IBLScene.h"
#include "GeoCullingScene.h"
#include "SSRScene.h"
#include "EmptyScene.h"

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
		/*case SCENE_MAIN_MENU:
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
			*/
		case SCENE_PARTICLES:
			newScene = new ParticleScene(this);
			break;
		case SCENE_AI:
			newScene = new AIScene(this);
			break;
		case SCENE_IBL:
			newScene = new IBLScene(this);
			break;
		case SCENE_GEO_CULLING:
			newScene = new GeoCullingScene(this);
			break;
		case SCENE_SSR:
			newScene = new SSRScene(this);
			break;
		case SCENE_NONE:
			newScene = new EmptyScene(this);
			break;
		default:
			newScene = new EmptyScene(this);
			break;
		}
		std::cout << "Scene '" << sceneType << "' created" << std::endl;
		scene = newScene;
		scene->OnSceneCreated();
	}

	void GameSceneManager::StartMenu()
	{

	}

	void GameSceneManager::StartNewGame()
	{
		ChangeScene(SCENE_PARTICLES);
	}
}