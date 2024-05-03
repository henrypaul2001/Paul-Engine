#pragma once
#include "Component.h"
#include "UIElement.h"
namespace Engine {
	enum CanvasTypes {
		SCREEN_SPACE,
		WORLD_SPACE
	};

	class ComponentUICanvas : public Component
	{
	public:
		ComponentUICanvas(CanvasTypes type);
		~ComponentUICanvas();

		ComponentTypes ComponentType() override { return COMPONENT_UICANVAS; }

		const CanvasTypes UIType() const { return uiType; }
		const std::vector<UIElement*>& UIElements() const { return uiElements; }

		void AddUIElement(UIElement* uiElement) { uiElements.push_back(uiElement); }
		UIElement* RemoveGetUIElement(int uiIndex) {
			UIElement* result = nullptr;
			if (uiIndex < uiElements.size()) {
				result = uiElements[uiIndex];
				uiElements.erase(uiElements.begin() + uiIndex);
			}
			return result;
		}
	private:
		std::vector<UIElement*> uiElements;

		CanvasTypes uiType;
	};
}