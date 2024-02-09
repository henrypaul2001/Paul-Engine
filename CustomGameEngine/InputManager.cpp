#include "InputManager.h"
namespace Engine {
	void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_RELEASE) {
			keyboard_Release(key, scancode, action, mods);
		}
		else if (action == GLFW_PRESS) {
			keyboard_Press(key, scancode, action, mods);
		}
	}

	InputManager::InputManager()
	{
		ClearInputs();

		// Allow a C++ member function to be used by the C, OpenGl api callback for key inputs
		glfwSetWindowUserPointer(glfwGetCurrentContext(), this);
		auto func = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			static_cast<InputManager*>(glfwGetWindowUserPointer(window))->key_callback(window, key, scancode, action, mods);
		};

		glfwSetKeyCallback(glfwGetCurrentContext(), func);
	}

	InputManager::~InputManager()
	{

	}

	void InputManager::ClearInputs()
	{
		for (int i = 0; i < 349; i++) {
			keysPressed[i] = false;
		}
	}

	void InputManager::keyboard_Press(int key, int scancode, int action, int mods) {
		keysPressed[key] = true;
		keyDown(key);
	}

	void InputManager::keyboard_Release(int key, int scancode, int action, int mods) {
		keysPressed[key] = false;
		keyUp(key);
	}
}