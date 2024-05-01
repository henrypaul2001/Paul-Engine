#pragma once
#include "Component.h"
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

		CanvasTypes UIType() { return uiType; }
		//std::vector<UIElement>& UIElements() { return uiElements; }
	private:
		//std::vector<UIElement> uiElements;

		CanvasTypes uiType;
	};
}