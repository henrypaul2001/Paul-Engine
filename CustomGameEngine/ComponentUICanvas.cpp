#include "ComponentUICanvas.h"
namespace Engine {
	ComponentUICanvas::ComponentUICanvas(const ComponentUICanvas& old_component)
	{
		this->owner = nullptr;

		this->uiType = old_component.uiType;

		// Copy UI elements
		this->uiElements.reserve(old_component.uiElements.size());
		for (int i = 0; i < old_component.uiElements.size(); i++) {
			uiElements.push_back(new UIElement(*old_component.uiElements[i]));
		}
	}

	ComponentUICanvas::ComponentUICanvas(CanvasTypes type)
	{
		uiType = type;
	}

	ComponentUICanvas::~ComponentUICanvas()
	{
		for (UIElement* uiElement : uiElements) {
			delete uiElement;
		}
	}

	void ComponentUICanvas::Close()
	{

	}
}