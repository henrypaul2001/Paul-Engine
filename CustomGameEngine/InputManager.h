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

		const glm::vec2& GetMousePos() const { return mousePosition; }
		void ClearInputs();
		void keyboard_Press(int key, int scancode, int action, int mods);
		void keyboard_Release(int key, int scancode, int action, int mods);
		virtual void ProcessInputs() = 0;
		virtual void Close() = 0;
		void SetCameraPointer(Camera* camPointer) { camera = camPointer; }
	private:
		bool firstMouse;
		float lastMouseX;
		float lastMouseY;

		glm::vec2 mousePosition;
	protected:
		Camera* camera;
		bool keysPressed[349];
		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;
	};
}
