#pragma once
#include "SceneManager.h"
namespace Engine
{
	class GameSceneManager : public SceneManager
	{
	protected:
		void ChangeScene(SceneTypes sceneType) override;
	public:
		GameSceneManager(int width, int height, int windowXPos, int windowYPos);
		~GameSceneManager();

		void StartMenu() override;
		void StartNewGame() override;
	};
}