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
		void SetMouseDownCallback(void(*leftMouseDown)()) { OnMouseDown = leftMouseDown; }
		void SetMouseUpCallback(void(*mouseUp)()) { OnMouseUp = mouseUp; }

		const bool IsHovering() const { return isHovering; }
		const bool IsMouseDown() const { return isMouseDown; }

		void MouseEnter() { isHovering = true; OnMouseEnter(); }
		void MouseExit() { isHovering = false; isMouseDown = false; OnMouseExit(); }
		void MouseDown() { isMouseDown = true; OnMouseDown(); }
		void MouseUp() { isMouseDown = false; OnMouseUp(); }
	protected:
		void (*OnMouseEnter)();
		void (*OnMouseExit)();
		void (*OnMouseDown)();
		void (*OnMouseUp)();

		bool isHovering;
		bool isMouseDown;
	};
}