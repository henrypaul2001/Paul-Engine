#pragma once
#include <GLFW/glfw3.h>
#include "SceneManager.h"

namespace Engine {
	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		static InputManager* instance;

		void ClearInputs();
		void keyboard_Press(int key, int scancode, int action, int mods);
		void keyboard_Release(int key, int scancode, int action, int mods);
		virtual void ProcessInputs() = 0;
		virtual void Close() = 0;
	private:
	protected:
		bool* keysPressed = new bool[255];
		SceneManager* sceneManager;
		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;
	};
}
