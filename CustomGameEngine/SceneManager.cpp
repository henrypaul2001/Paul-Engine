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

		const double targetFrameTime = (1.0 / 60.0) / 1.2;
		auto startTime = std::chrono::high_resolution_clock::now();
		auto previousTime = startTime;
		auto currentTime = startTime;
		
		while (true)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> timeStep = currentTime - previousTime;
			//std::chrono::duration<float, std::milli> duration = currentTime - previousTime;
			Scene::dt = std::chrono::duration_cast<std::chrono::milliseconds>(timeStep).count() / 1000.0;

			// Process inputs


			// Update scene
			OnUpdateFrame();

			// Render scene
			OnRenderFrame();

			// Poll events

			// Prepare for next frame

			previousTime = currentTime;
			//std::cout << "FPS: " << (1.0f / Scene::dt) << std::endl;

			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> frameDuration = endTime - currentTime;
			std::chrono::duration<double> remainingTime = std::chrono::duration<double>(targetFrameTime) - frameDuration;

			if (remainingTime.count() > 0) {
				std::this_thread::sleep_for(remainingTime);
			}
			/*
			auto nextFrameTime = currentTime + std::chrono::nanoseconds(16666667);
			// Sleep until nextFrameTime
			auto sleepDuration = nextFrameTime - std::chrono::high_resolution_clock::now();
			if (sleepDuration > std::chrono::nanoseconds(0)) {
				std::this_thread::sleep_for(sleepDuration);
			}
			*/
		}
	}

	void SceneManager::UpdateKeyCallback(GLFWwindow* window, GLFWkeyfun callback)
	{
		glfwSetKeyCallback(window, callback);
	}
}