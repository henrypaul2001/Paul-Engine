#include "InputManager.h"
namespace Engine {
	InputManager::InputManager()
	{
		glfwSetKeyCallback(glfwGetCurrentContext(), key_callback);
		instance = this;
	}

	InputManager::~InputManager()
	{
		delete[] keysPressed;
	}

	void InputManager::ClearInputs()
	{
		delete[] keysPressed;
		keysPressed = new bool[255];
	}

	void InputManager::keyboard_Press(int key, int scancode, int action, int mods) {
		keysPressed[key] = false;
		keyUp(key);
	}

	void InputManager::keyboard_Release(int key, int scancode, int action, int mods) {
		keysPressed[key] = true;
		keyDown(key);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_RELEASE) {
			InputManager::instance->keyboard_Release(key, scancode, action, mods);
		}
		else if (action == GLFW_PRESS) {
			InputManager::instance->keyboard_Press(key, scancode, action, mods);
		}
	}
}