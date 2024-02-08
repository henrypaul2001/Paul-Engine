#pragma once
#include "InputManager.h"
namespace Engine {
	class GameInputManager : public InputManager
	{
	public:
		GameInputManager();
		~GameInputManager();
		void ProcessInputs() override;
		void Close() override;
	private:
	protected:
		void keyUp(int key) override;
		void keyDown(int key) override;
	};
}