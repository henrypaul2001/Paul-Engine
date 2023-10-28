#pragma once
#include "SceneManager.h"
namespace Engine 
{
	enum SceneTypes {
		SCENE_NONE,
		SCENE_MAIN_MENU,
		SCENE_GAME,
		SCENE_GAME_OVER,
		SCENE_WIN
	};

	class Scene
	{
	protected:
		SceneManager* sceneManager;
	public:
		static float dt;
		Scene(SceneManager& sceneManager);
		~Scene();

		virtual void Render() = 0;
		virtual void Update() = 0;
		virtual void Close() = 0;
	};
}

