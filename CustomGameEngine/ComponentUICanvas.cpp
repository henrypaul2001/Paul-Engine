#include "ComponentUICanvas.h"
namespace Engine {
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