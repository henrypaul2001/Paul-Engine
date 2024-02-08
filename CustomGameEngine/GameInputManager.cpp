#include "GameInputManager.h"
#include <iostream>

namespace Engine {
	Engine::GameInputManager::GameInputManager()
	{

	}

	Engine::GameInputManager::~GameInputManager()
	{
		Close();
	}

	void Engine::GameInputManager::ProcessInputs()
	{
		if (keysPressed[GLFW_KEY_W]) {
			std::cout << "GAMEINPUTMANAGER::KEYCHECK::W::TRUE" << std::endl;
		}
		if (keysPressed[GLFW_KEY_S]) {
			std::cout << "GAMEINPUTMANAGER::KEYCHECK::S::TRUE" << std::endl;
		}
	}

	void Engine::GameInputManager::Close()
	{
		// clear scene manager delegates
	}

	void Engine::GameInputManager::keyUp(int key)
	{

	}

	void Engine::GameInputManager::keyDown(int key)
	{

	}
}