#pragma once
#include <iostream>
#include "Camera.h"
namespace Engine {
	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		const glm::vec2& GetMousePos() const { return mousePosition; }
		void ClearInputs();
		void keyboard_Press(int key, int scancode, int action, int mods);
		void keyboard_Release(int key, int scancode, int action, int mods);
		void mouse_Press(int button, int action, int mods);
		void mouse_Release(int button, int action, int mods);
		virtual void ProcessInputs() = 0;
		virtual void Close() = 0;
		void SetCameraPointer(Camera* camPointer) { camera = camPointer; }

		void SetCursorLock(bool locked);
		bool GetCursorLock() { return cursorLocked; }

		const bool GetLeftMouseDown() const { return leftMouseDown; }
	private:
		bool firstMouse;
		float lastMouseX;
		float lastMouseY;

		glm::vec2 mousePosition;
		bool leftMouseDown;
	protected:
		bool cursorLocked;

		Camera* camera;
		bool keysPressed[349];
		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;

		virtual void mouseDown(int button) = 0;
		virtual void mouseUp(int button) = 0;
	};
}
