#include "GameInputManager.h"
namespace Engine {
	GameInputManager::GameInputManager()
	{

	}

	GameInputManager::~GameInputManager()
	{
		Close();
	}

	void GameInputManager::ProcessInputs()
	{
		if (keysPressed[GLFW_KEY_ESCAPE]) {
			std::cout << "GAMEINPUTMANAGER::KEYCHECK::ESCAPE::TRUE" << std::endl;
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}
		if (keysPressed[GLFW_KEY_W]) {
			std::cout << "GAMEINPUTMANAGER::KEYCHECK::W::TRUE" << std::endl;
		}
		if (keysPressed[GLFW_KEY_S]) {
			std::cout << "GAMEINPUTMANAGER::KEYCHECK::S::TRUE" << std::endl;
		}
	}

	void GameInputManager::Close()
	{
		// clear scene manager delegates
	}

	void GameInputManager::keyUp(int key)
	{
		std::cout << "Key num: " << key << "| UP" << std::endl;
	}

	void GameInputManager::keyDown(int key)
	{
		std::cout << "Key num: " << key << "| DOWN" << std::endl;
	}
}