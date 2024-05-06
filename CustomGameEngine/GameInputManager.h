#pragma once
#include "InputManager.h"
#include "GameScene.h"
namespace Engine {
	class GameInputManager : public InputManager
	{
	public:
		GameInputManager(Scene* owner);
		~GameInputManager();
		void ProcessInputs() override;
		void Close() override;
	private:
	protected:
		void keyUp(int key) override;
		void keyDown(int key) override;
		void mouseUp(int button) override;
		void mouseDown(int button) override;
		Scene* owner;
	};
}