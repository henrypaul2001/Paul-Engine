#include "UIButton.h"
namespace Engine {
	UIButton::UIButton(glm::vec2 position, glm::vec2 scale, Shader* shader, int idTag) : UIElement(position, scale, shader)
	{
		OnMouseEnter = nullptr;
		OnMouseExit = nullptr;
		OnMouseDown = nullptr;
		OnMouseUp = nullptr;
		type = UI_BUTTON;
		isHovering = false;
		isMouseDown = false;
		this->idTag = idTag;
		buttonScale = scale;
	}

	UIButton::UIButton(glm::vec2 position, glm::vec2 scale, Shader* shader, int idTag, UIBackground background) : UIElement(position, scale, shader, background)
	{
		OnMouseEnter = nullptr;
		OnMouseExit = nullptr;
		OnMouseDown = nullptr;
		OnMouseUp = nullptr;
		type = UI_BUTTON;
		isHovering = false;
		isMouseDown = false;
		this->idTag = idTag;
		buttonScale = scale;
	}

	UIButton::~UIButton()
	{

	}
}