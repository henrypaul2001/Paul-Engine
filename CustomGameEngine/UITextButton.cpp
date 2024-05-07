#include "UITextButton.h"
#include "ResourceManager.h"
namespace Engine {
	UITextButton::UITextButton(std::string text, glm::vec2 position, glm::vec2 textScale, glm::vec2 buttonScale, TextFont* font, glm::vec3 colour, Shader* shader) : UIElement(position, textScale, shader), UIText(text, position, textScale, font, colour), UIButton(position, buttonScale, shader)
	{
		buttonType = BUTTON_TEXT;
	}

	UITextButton::UITextButton(std::string text, glm::vec2 position, glm::vec2 textScale, glm::vec2 buttonScale, TextFont* font, glm::vec3 colour) : UIElement(position, textScale, ResourceManager::GetInstance()->DefaultTextShader()), UIText(text, position, textScale, font, colour, ResourceManager::GetInstance()->DefaultTextShader()), UIButton(position, buttonScale, ResourceManager::GetInstance()->DefaultTextShader())
	{
		buttonType = BUTTON_TEXT;
	}

	UITextButton::~UITextButton()
	{

	}

	void UITextButton::Draw()
	{
		UIText::Draw();
	}
}