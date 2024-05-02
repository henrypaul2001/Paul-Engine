#pragma once
#include "UIElement.h"
#include "TextFont.h"
namespace Engine {
	class UIText : public UIElement
	{
	public:
		UIText(glm::vec2 position, TextFont* font);
		~UIText();

		unsigned int TextVAO() { return textVAO; }
		unsigned int TextVBO() { return textVBO; }
	private:
		void InitTextVAOVBO();

		TextFont* font;

		unsigned int textVAO;
		unsigned int textVBO;
	};
}