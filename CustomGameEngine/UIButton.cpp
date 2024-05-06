#include "UIButton.h"
namespace Engine {
	UIButton::UIButton(glm::vec2 position, glm::vec2 scale, Shader* shader) : UIElement(position, scale, shader)
	{
		OnMouseEnter = nullptr;
		OnMouseExit = nullptr;
		OnMouseDown = nullptr;
		OnMouseUp = nullptr;
	}

	UIButton::~UIButton()
	{

	}
}