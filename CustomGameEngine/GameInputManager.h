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

		Scene* owner;
	};
}