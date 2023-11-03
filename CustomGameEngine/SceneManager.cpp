#include "SceneManager.h"
#include "Scene.h"
#include <chrono>
#include <thread>
#include <iostream>
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
		OnLoadTemp();
	}

	SceneManager::~SceneManager() 
	{

	}

	void SceneManager::OnLoadTemp() 
	{
		// OpenGL setup

		// Load GUI

		//gameIsRunning = true;
		std::cout << "Onload\n";
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
		std::cout << "Starting new game\n";
		StartNewGame();

		const double targetFrameTime = 1.0 / 60.0;
		auto startTime = std::chrono::high_resolution_clock::now();
		auto previousTime = startTime;
		auto currentTime = startTime;
		//currentTime - startTime).count() < std::chrono::seconds(10).count()
		while (true)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> duration = currentTime - previousTime;
			Scene::dt = duration.count();

			OnUpdateFrame();
			OnRenderFrame();

			double sleepTime = targetFrameTime - (std::chrono::high_resolution_clock::now() - currentTime).count();
			if (sleepTime > 0) {
				std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
			}
		}
	}
}