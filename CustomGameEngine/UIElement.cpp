#include "UIElement.h"
namespace Engine {
	UIElement::UIElement(glm::vec2 position, glm::vec2 scale, Shader* shader)
	{
		this->position = position;
		this->scale = scale;
		this->shader = shader;
	}

	UIElement::~UIElement()
	{
	}
}