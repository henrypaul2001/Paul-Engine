#pragma once
#include "SystemNew.h"
#include "UIButton.h"
#include "ComponentUICanvas.h"
#include "InputManager.h"
namespace Engine {
	class SystemUIMouseInteraction : public SystemNew
	{
	public:
		SystemUIMouseInteraction(EntityManagerNew* ecs, InputManager** inputManager) : SystemNew(ecs), inputManager(inputManager) {}
		~SystemUIMouseInteraction() {}

		constexpr const char* SystemName() override { return "SYSTEM_UI_MOUSE_INTERACTION"; }

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentUICanvas& canvas);
		void AfterAction();
	private:
		void ProcessUIButton(UIButton* button, ComponentTransform& canvasTransform) const;
		InputManager** inputManager;
	};
}