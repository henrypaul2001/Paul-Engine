#pragma once
#include "UIElement.h"
namespace Engine {
	enum ButtonTypes {
		BUTTON_TEXT,
		BUTTON_IMAGE,
	};

	class UIButton : virtual public UIElement
	{
	public:
		UIButton(glm::vec2 position, glm::vec2 scale, Shader* shader);
		~UIButton();

		virtual void Draw() override = 0;

		void SetMouseEnterCallback(void(*mouseEnter)(UIButton* button)) { OnMouseEnter = mouseEnter; }
		void SetMouseExitCallback(void(*mouseExit)(UIButton* button)) { OnMouseExit = mouseExit; }
		void SetMouseDownCallback(void(*leftMouseDown)(UIButton* button)) { OnMouseDown = leftMouseDown; }
		void SetMouseUpCallback(void(*mouseUp)(UIButton* button)) { OnMouseUp = mouseUp; }

		const bool IsHovering() const { return isHovering; }
		const bool IsMouseDown() const { return isMouseDown; }

		void MouseEnter() { isHovering = true; OnMouseEnter(this); }
		void MouseExit() { isHovering = false; isMouseDown = false; OnMouseExit(this); }
		void MouseDown() { isMouseDown = true; OnMouseDown(this); }
		void MouseUp() { isMouseDown = false; OnMouseUp(this); }

		const glm::vec2& GetButtonScale() const { return buttonScale; }
		const ButtonTypes& GetButtonType() const { return buttonType; }
	protected:
		void (*OnMouseEnter)(UIButton* button);
		void (*OnMouseExit)(UIButton* button);
		void (*OnMouseDown)(UIButton* button);
		void (*OnMouseUp)(UIButton* button);

		bool isHovering;
		bool isMouseDown;

		glm::vec2 buttonScale;

		ButtonTypes buttonType;
	};
}