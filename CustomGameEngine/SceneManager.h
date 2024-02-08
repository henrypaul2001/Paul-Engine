#pragma once
#include <functional>
#include "Scene.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
namespace Engine
{
	enum SceneTypes {
		SCENE_NONE,
		SCENE_MAIN_MENU,
		SCENE_GAME,
		SCENE_GAME_OVER,
		SCENE_WIN
	};

	using SceneDelegate = std::function<void()>;
	using MouseDelegate = std::function<void()>;

	class SceneManager
	{
	protected:
		void OnLoad();
		void OnUpdateFrame();
		void OnRenderFrame();

		//bool gameIsRunning = false;
	public:
		SceneManager(int width, int height, int windowXPos, int windowYPos);
		~SceneManager();

		static int SCR_WIDTH;
		static int SCR_HEIGHT;
		static int windowXPos;
		static int windowYPos;

		Scene* scene;

		SceneDelegate renderer;
		SceneDelegate updater;

		GLFWwindow* window;

		MouseDelegate mouseDelegate;

		void UpdateKeyCallback(GLFWwindow* window, GLFWkeyfun callback);

		virtual void StartNewGame() = 0;
		virtual void StartMenu() = 0;
		virtual void ChangeScene(SceneTypes sceneType) = 0;

		static int& getWindowHeight() { return SCR_HEIGHT; }
		static int& getWindowWidth() { return SCR_WIDTH; }

		void Run();
	};
}

