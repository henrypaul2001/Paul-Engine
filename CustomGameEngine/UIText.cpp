#include "UIText.h"
#include <glad/glad.h>
namespace Engine {
	UIText::UIText(glm::vec2 position, glm::vec2 scale, TextFont* font, glm::vec3 colour) : UIElement(position, scale)
	{
		this->font = font;
		this->textColour = colour;
		textVAO = 0;
		textVBO = 0;
		InitTextVAOVBO();
	}

	UIText::~UIText()
	{

	}

	void UIText::InitTextVAOVBO()
	{
		glGenVertexArrays(1, &textVAO);
		glGenBuffers(1, &textVBO);
		glBindVertexArray(textVAO);
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}