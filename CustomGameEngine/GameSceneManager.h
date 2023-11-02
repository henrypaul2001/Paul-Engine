#pragma once
#include "SceneManager.h"
namespace Engine
{
	class GameSceneManager : public SceneManager
	{
	public:
		GameSceneManager(int width, int height, int windowXPos, int windowYPos);
		~GameSceneManager();

		void ChangeScene(SceneTypes sceneType) override;
		void StartMenu() override;
		void StartNewGame() override;
	};
}