#pragma once
#include "UIElement.h"
#include "TextFont.h"
#include <string>
#include <glm/ext/vector_float3.hpp>
namespace Engine {
	class UIText : public UIElement
	{
	public:
		UIText(glm::vec2 position, glm::vec2 scale, TextFont* font, glm::vec3 colour);
		~UIText();

		const unsigned int TextVAO() const { return textVAO; }
		const unsigned int TextVBO() const { return textVBO; }

		const std::string& Text() const { return text; }

		void SetText(const std::string& newText) { text = newText; }
		void AppendText(const std::string& text) { this->text += text; }

		void SetColour(const glm::vec3& newColour) { textColour = newColour; }
		const glm::vec3& TextColour() const { return textColour; }
	private:
		void InitTextVAOVBO();

		TextFont* font;
		std::string text;

		glm::vec3 textColour;

		unsigned int textVAO;
		unsigned int textVBO;
	};
}