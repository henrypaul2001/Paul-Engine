#include "UIElement.h"
namespace Engine {
	UIElement::UIElement(glm::vec2 position, glm::vec2 scale)
	{
		this->position = position;
		this->scale = scale;
	}

	UIElement::~UIElement()
	{
	}
}