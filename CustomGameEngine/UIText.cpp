#include "UIText.h"
#include <glad/glad.h>
#include "ResourceManager.h"
namespace Engine {
	UIText::UIText(std::string text, glm::vec2 position, glm::vec2 scale, TextFont* font, glm::vec3 colour) : UIElement(position, scale, ResourceManager::GetInstance()->DefaultTextShader())
	{
		this->text = text;
		this->font = font;
		this->textColour = colour;
		textVAO = 0;
		textVBO = 0;
		type = UI_TEXT;
		InitTextVAOVBO();
	}

	UIText::UIText(std::string text, glm::vec2 position, glm::vec2 scale, TextFont* font, glm::vec3 colour, Shader* shader) : UIElement(position, scale, shader)
	{
		this->text = text;
		this->font = font;
		this->textColour = colour;
		textVAO = 0;
		textVBO = 0;
		type = UI_TEXT;
		InitTextVAOVBO();
	}

	UIText::~UIText()
	{

	}

	void UIText::Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale)
	{
		shader->Use();
		shader->setVec3("textColour", textColour);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(textVAO);

		glm::vec2 transformedPosition = position + canvasPosition;
		glm::vec2 transformedScale = scale * canvasScale;

		float xCharacterPos = transformedPosition.x;
		//glEnable(GL_BLEND);
		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++) {
			const TextCharacter* ch = font->GetCharacter(*c);

			float xPos = xCharacterPos + ch->Bearing.x * transformedScale.x;
			float yPos = transformedPosition.y - (ch->Size.y - ch->Bearing.y) * transformedScale.y;

			float width = ch->Size.x * transformedScale.x;
			float height = ch->Size.y * transformedScale.y;

			// this will change. Instead, use transformations in shader
			// --------------------------------------------------------
			// update VBO for each character
			float vertices[6][4] = {
				{ xPos, yPos + height, 0.0f, 0.0f },
				{ xPos, yPos, 0.0f, 1.0f },
				{ xPos + width, yPos, 1.0f, 1.0f },
				{ xPos, yPos + height, 0.0f, 0.0f },
				{ xPos + width, yPos, 1.0f, 1.0f },
				{ xPos + width, yPos + height, 1.0f, 0.0f }
			};

			// (look into using a texture atlas instead of unique textures per character)
			// --------------------------------------------------------------------------
			// bind character glyph texture
			glBindTexture(GL_TEXTURE_2D, ch->TextureID);

			// update VBO
			glBindBuffer(GL_ARRAY_BUFFER, textVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);

			xCharacterPos += (ch->Advance >> 6) * scale.x;
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
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