// rip Tango Gameworks
#pragma once
#include "UIText.h"
#include "UIButton.h"
namespace Engine {
	class UITextButton : public UIText, public UIButton
	{
	public:
		UITextButton(std::string text, glm::vec2 position, glm::vec2 textScale, glm::vec2 buttonScale, TextFont* font, glm::vec3 colour, Shader* shader, int idTag);
		UITextButton(std::string text, glm::vec2 position, glm::vec2 textScale, glm::vec2 buttonScale, TextFont* font, glm::vec3 colour, int idTag);
		~UITextButton();

		void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) override;
	private:
	};
}