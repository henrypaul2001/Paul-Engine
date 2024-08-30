#pragma once
#include <functional>
#include "Scene.h"
#include <thread>
#include <iostream>
#include "AudioManager.h"
namespace Engine
{
	enum SceneTypes {
		SCENE_NONE,
		SCENE_MAIN_MENU,
		SCENE_GAME,
		SCENE_GAME_OVER,
		SCENE_WIN,
		SCENE_SPONZA,
		SCENE_AO,
		SCENE_PBR,
		SCENE_COLLISIONS,
		SCENE_PHYSICS,
		SCENE_INSTANCED,
		SCENE_ANIMATION,
		SCENE_AUDIO,
		SCENE_PARTICLES,
		SCENE_AI,
		SCENE_IBL,
		SCENE_GEO_CULLING,
	};

	using SceneDelegate = std::function<void()>;

	class SceneManager
	{
	protected:
		void OnLoad();
		void OnUpdateFrame();
		void OnRenderFrame();

		ResourceManager* resources;
		AudioManager* audioManager;

		int SCR_WIDTH;
		int SCR_HEIGHT;
		int windowXPos;
		int windowYPos;

		SceneTypes changeSceneAtEndOfFrame;
		virtual void ChangeScene(SceneTypes sceneType) = 0;

		GLFWwindow* window;
	public:
		SceneManager(int width, int height, int windowXPos, int windowYPos);
		~SceneManager();
		
		Scene* scene;

		SceneDelegate renderer;
		SceneDelegate updater;

		void ChangeSceneAtEndOfFrame(SceneTypes scene) { changeSceneAtEndOfFrame = scene; }

		virtual void StartNewGame() = 0;
		virtual void StartMenu() = 0;

		int GetWindowHeight() { return SCR_HEIGHT; }
		int GetWindowWidth() { return SCR_WIDTH; }

		const GLFWwindow* GetWindow() const { return window; }

		void Run();
	};
}

