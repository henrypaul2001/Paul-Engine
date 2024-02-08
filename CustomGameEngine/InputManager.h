#pragma once
#include <GLFW/glfw3.h>

namespace Engine {
	class InputManager
	{
	public:
		void ClearInputs();
		void key_callback(GLFWwindow* window, int key, int scancode, int mods);
		void Keyboard_KeyDown(int key);
		void Keyboard_KeyUp(int key);
		virtual void ProcessInputs() = 0;
		virtual void Close() = 0;
	private:
	protected:
		bool keysPressed[255];
		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;
	};
}
