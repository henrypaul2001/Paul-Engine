#pragma once
#include "UIElement.h"
#include <vector>
namespace Engine {
	enum CanvasTypes {
		SCREEN_SPACE,
		WORLD_SPACE
	};

	class ComponentUICanvas
	{
	public:
		ComponentUICanvas(const ComponentUICanvas& old_component);
		ComponentUICanvas(CanvasTypes type);
		~ComponentUICanvas();

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