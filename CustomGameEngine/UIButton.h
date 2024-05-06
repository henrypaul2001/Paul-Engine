#pragma once
#include "UIElement.h"
namespace Engine {
	class UIButton : public UIElement
	{
	public:
		UIButton(glm::vec2 position, glm::vec2 scale, Shader* shader);
		~UIButton();

		virtual void Draw() override = 0;

		void SetMouseEnterCallback(void(*mouseEnter)()) { OnMouseEnter = mouseEnter; }
		void SetMouseExitCallback(void(*mouseExit)()) { OnMouseExit = mouseExit; }
		void SetMouseDownCallback(void(*mouseDown)()) { OnMouseDown = mouseDown; }
		void SetMouseUpCallback(void(*mouseUp)()) { OnMouseUp = mouseUp; }
	protected:
		void (*OnMouseEnter)();
		void (*OnMouseExit)();
		void (*OnMouseDown)();
		void (*OnMouseUp)();
	};
}