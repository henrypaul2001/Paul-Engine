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

	void InputManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		camera->ProcessMouseScroll(yoffset);
	}

	void InputManager::mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		if (firstMouse) {
			lastMouseX = xpos;
			lastMouseY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastMouseX;
		float yoffset = lastMouseY - ypos;
		lastMouseX = xpos;
		lastMouseY = ypos;

		mousePosition.x = lastMouseX;
		mousePosition.y = lastMouseY;

		//std::cout << mousePosition.x << " | " << mousePosition.y << std::endl;

		if (cursorLocked) {
			camera->ProcessMouseMovement(xoffset, yoffset, true);
		}
	}

	void InputManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if (action == GLFW_RELEASE) {
			mouse_Release(button, action, mods);
		}
		else if (action == GLFW_PRESS) {
			mouse_Press(button, action, mods);
		}
	}

	InputManager::InputManager()
	{
		ClearInputs();
		firstMouse = true;
		lastMouseX = 400;
		lastMouseY = 300;

		// Allow a C++ member function to be used by the C, OpenGl api callback for input callbacks
		glfwSetWindowUserPointer(glfwGetCurrentContext(), this);
		auto key = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			static_cast<InputManager*>(glfwGetWindowUserPointer(window))->key_callback(window, key, scancode, action, mods);
		};

		auto scroll = [](GLFWwindow* window, double xoffset, double yoffset) {
			static_cast<InputManager*>(glfwGetWindowUserPointer(window))->scroll_callback(window, xoffset, yoffset);
		};

		auto mouse = [](GLFWwindow* window, double xpos, double ypos) {
			static_cast<InputManager*>(glfwGetWindowUserPointer(window))->mouse_callback(window, xpos, ypos);
		};

		auto mouse_button = [](GLFWwindow* window, int button, int action, int mods) {
			static_cast<InputManager*>(glfwGetWindowUserPointer(window))->mouse_button_callback(window, button, action, mods);
		};

		glfwSetKeyCallback(glfwGetCurrentContext(), key);
		glfwSetScrollCallback(glfwGetCurrentContext(), scroll);
		glfwSetCursorPosCallback(glfwGetCurrentContext(), mouse);
		glfwSetMouseButtonCallback(glfwGetCurrentContext(), mouse_button);

		leftMouseDown = false;
		cursorLocked = true;
		SetCursorLock(cursorLocked);
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

	void InputManager::mouse_Press(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			leftMouseDown = true;
		}
		mouseDown(button);
	}

	void InputManager::mouse_Release(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			leftMouseDown = false;
		}
		mouseUp(button);
	}

	void InputManager::SetCursorLock(bool locked)
	{
		cursorLocked = locked;

		if (cursorLocked) {
			std::cout << "INPUTMANAGER::ENABLE::Cursor lock" << std::endl;
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			std::cout << "INPUTMANAGER::DISABLE::Cursor lock" << std::endl;
			glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}