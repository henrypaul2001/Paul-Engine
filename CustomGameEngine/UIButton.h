#pragma once
#include "UIElement.h"
#include <functional>
namespace Engine {
	enum ButtonTypes {
		BUTTON_TEXT,
		BUTTON_IMAGE,
	};

	class UIButton : virtual public UIElement
	{
	public:
		UIButton(glm::vec2 position, glm::vec2 scale, Shader* shader, int idTag);
		~UIButton();

		virtual void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) override = 0;

		void SetMouseEnterCallback(std::function<void(UIButton*)> mouseEnter) { OnMouseEnter = mouseEnter; }
		void SetMouseExitCallback(std::function<void(UIButton*)> mouseExit) { OnMouseExit = mouseExit; }
		void SetMouseDownCallback(std::function<void(UIButton*)> mouseDown) { OnMouseDown = mouseDown; }
		void SetMouseUpCallback(std::function<void(UIButton*)> mouseUp) { OnMouseUp = mouseUp; }
		void SetMouseHoldCallback(std::function<void(UIButton*)> mouseHold) { OnMouseHold = mouseHold; }

		const bool IsHovering() const { return isHovering; }
		const bool IsMouseDown() const { return isMouseDown; }

		void MouseEnter() { isHovering = true; if (OnMouseEnter != nullptr) { OnMouseEnter(this); } }
		void MouseExit() { isHovering = false; isMouseDown = false; if (OnMouseExit != nullptr) { OnMouseExit(this); } }
		void MouseDown() { isMouseDown = true; if (OnMouseDown != nullptr) { OnMouseDown(this); } }
		void MouseUp() { isMouseDown = false; if (OnMouseUp != nullptr) { OnMouseUp(this); } }
		void MouseHold() { if (OnMouseHold != nullptr) { OnMouseHold(this); } }

		void SetButtonScale(const glm::vec2 newBtnScale) { buttonScale = newBtnScale; }
		void SetIDTag(const int tag) { idTag = tag; }

		const glm::vec2& GetButtonScale() const { return buttonScale; }
		const ButtonTypes& GetButtonType() const { return buttonType; }
		const int GetIDTag() const { return idTag; }
	protected:
		std::function<void(UIButton*)> OnMouseEnter;
		std::function<void(UIButton*)> OnMouseExit;
		std::function<void(UIButton*)> OnMouseDown;
		std::function<void(UIButton*)> OnMouseUp;
		std::function<void(UIButton*)> OnMouseHold;

		bool isHovering;
		bool isMouseDown;

		glm::vec2 buttonScale;

		ButtonTypes buttonType;

		int idTag;
	};
}