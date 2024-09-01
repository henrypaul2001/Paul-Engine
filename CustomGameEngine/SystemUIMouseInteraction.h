#pragma once
#include "System.h"
#include "UIButton.h"
#include "ComponentUICanvas.h"
#include "InputManager.h"
namespace Engine {
	class SystemUIMouseInteraction : public System
	{
	public:
		SystemUIMouseInteraction(InputManager* inputManager);
		~SystemUIMouseInteraction();

		SystemTypes Name() override { return SYSTEM_UI_INTERACT; }

		ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_UICANVAS);

		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	private:
		void ProcessUIButton(UIButton* button, ComponentTransform* canvasTransform) const;
		InputManager* inputManager;
	};
}