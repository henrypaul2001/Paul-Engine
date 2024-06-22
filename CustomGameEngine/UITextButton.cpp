#include "UITextButton.h"
#include "ResourceManager.h"
namespace Engine {
	UITextButton::UITextButton(std::string text, glm::vec2 position, glm::vec2 textScale, glm::vec2 buttonScale, TextFont* font, glm::vec3 colour, Shader* shader, int idTag) : UIElement(position, textScale, shader), UIText(text, position, textScale, font, colour), UIButton(position, buttonScale, shader, idTag)
	{
		buttonType = BUTTON_TEXT;
	}

	UITextButton::UITextButton(std::string text, glm::vec2 position, glm::vec2 textScale, glm::vec2 buttonScale, TextFont* font, glm::vec3 colour, int idTag) : UIElement(position, textScale, ResourceManager::GetInstance()->DefaultTextShader()), UIText(text, position, textScale, font, colour, ResourceManager::GetInstance()->DefaultTextShader()), UIButton(position, buttonScale, ResourceManager::GetInstance()->DefaultTextShader(), idTag)
	{
		buttonType = BUTTON_TEXT;
	}

	UITextButton::~UITextButton()
	{

	}

	void UITextButton::Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale)
	{
		UIText::Draw(canvasPosition, canvasScale);
	}
}