#include "GameInputManager.h"
#include "Scene.h"
namespace Engine {
	GameInputManager::GameInputManager(GameScene* owner)
	{
		this->owner = owner;
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
			camera->ProcessKeyboard(FORWARD, Scene::dt);
		}
		if (keysPressed[GLFW_KEY_S]) {
			camera->ProcessKeyboard(BACKWARD, Scene::dt);
		}
		if (keysPressed[GLFW_KEY_D]) {
			camera->ProcessKeyboard(RIGHT, Scene::dt);
		}
		if (keysPressed[GLFW_KEY_A]) {
			camera->ProcessKeyboard(LEFT, Scene::dt);
		}

		if (keysPressed[GLFW_KEY_LEFT_CONTROL] && keysPressed[GLFW_KEY_SPACE]) {
			camera->ProcessKeyboard(UP, Scene::dt);
		}
		else if (keysPressed[GLFW_KEY_SPACE]) {
			camera->ProcessKeyboard(UP_WORLD, Scene::dt);
		}

		if (keysPressed[GLFW_KEY_LEFT_CONTROL] && keysPressed[GLFW_KEY_LEFT_SHIFT]) {
			camera->ProcessKeyboard(DOWN, Scene::dt);
		}
		else if (keysPressed[GLFW_KEY_LEFT_SHIFT]) {
			camera->ProcessKeyboard(DOWN_WORLD, Scene::dt);
		}
	}

	void GameInputManager::Close()
	{
		// clear scene manager delegates
	}

	void GameInputManager::keyUp(int key)
	{
		std::cout << "Key num: " << key << "| UP" << std::endl;
		if (key == GLFW_KEY_SLASH) {
			owner->ChangePostProcessEffect();
		}
	}

	void GameInputManager::keyDown(int key)
	{
		std::cout << "Key num: " << key << "| DOWN" << std::endl;
	}
}