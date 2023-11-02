#pragma once
#include <functional>
#include "Scene.h"
namespace Engine
{
	using SceneDelegate = std::function<void()>;
	using KeyboardDelegate = std::function<void()>;
	using MouseDelegate = std::function<void()>;

	class SceneManager
	{
	protected:
		void OnLoad();
		void OnUpdateFrame();
		void OnRenderFrame();
	public:
		static int width;
		static int height;
		static int windowXPos;
		static int windowYPos;

		SceneDelegate renderer;
		SceneDelegate updater;

		KeyboardDelegate keyboardDownDelegate;
		KeyboardDelegate keyboardUpDelegate;

		MouseDelegate mouseDelegate;

		SceneManager(int width, int height, int windowXPos, int windowYPos);
		~SceneManager();

		virtual void StartNewGame() = 0;
		virtual void StartMenu() = 0;
		virtual void ChangeScene(SceneTypes& sceneType) = 0;

		static int& getWindowHeight() { return height; }
		static int& getWindowWidth() { return width; }
	};
}

