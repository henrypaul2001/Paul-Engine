#include "SceneManager.h"
#include "Scene.h"
#include <chrono>
#include <thread>
namespace Engine
{
	int SceneManager::width;
	int SceneManager::height;
	int SceneManager::windowXPos;
	int SceneManager::windowYPos;

	SceneManager::SceneManager(int width, int height, int windowXPos, int windowYPos) 
	{
		this->width = width;
		this->height = height;
		this->windowXPos = windowXPos;
		this->windowYPos = windowYPos;
		OnLoad();
	}

	SceneManager::~SceneManager() 
	{
		delete& renderer;
		delete& updater;
		delete& keyboardUpDelegate;
		delete& keyboardDownDelegate;
		delete& mouseDelegate;
	}

	void SceneManager::OnLoad() 
	{
		// OpenGL setup

		// Load GUI

		//gameIsRunning = true;
		StartNewGame();
	}

	void SceneManager::OnUpdateFrame() 
	{
		updater();
	}

	void SceneManager::OnRenderFrame() 
	{
		renderer();

		// GL flush
		// Swap buffers
	}

	void SceneManager::Run() 
	{
		// Temporary "game loop" standing in for future OpenGL game loop
		const double targetFrameTime = 1.0 / 60.0;
		auto startTime = std::chrono::high_resolution_clock::now();
		auto previousTime = startTime;
		auto currentTime = startTime;
		while ((currentTime - startTime).count() > 10.0)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			Scene::dt = (currentTime - previousTime).count();

			OnUpdateFrame();
			OnRenderFrame();

			double sleepTime = targetFrameTime - (std::chrono::high_resolution_clock::now() - currentTime).count();
			if (sleepTime > 0) {
				std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
			}
		}
	}
}