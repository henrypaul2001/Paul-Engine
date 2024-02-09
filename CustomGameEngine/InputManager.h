#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
namespace Engine {
	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

		void ClearInputs();
		void keyboard_Press(int key, int scancode, int action, int mods);
		void keyboard_Release(int key, int scancode, int action, int mods);
		virtual void ProcessInputs() = 0;
		virtual void Close() = 0;
	private:
	protected:
		bool keysPressed[349];
		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;
	};
}
