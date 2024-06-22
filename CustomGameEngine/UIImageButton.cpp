#include "UIImageButton.h"
namespace Engine {
	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, int idTag) : UIElement(position, imageScale, ResourceManager::GetInstance()->DefaultImageShader()), UIImage(position, imageScale, imageTexture), UIButton(position, buttonScale, ResourceManager::GetInstance()->DefaultImageShader(), idTag)
	{
		buttonType = BUTTON_IMAGE;
	}

	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, Shader* shader, int idTag) : UIElement(position, imageScale, shader), UIImage(position, imageScale, imageTexture, shader), UIButton(position, buttonScale, shader, idTag)
	{
		buttonType = BUTTON_IMAGE;
	}

	UIImageButton::~UIImageButton()
	{
	}

	void UIImageButton::Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale)
	{
		UIImage::Draw(canvasPosition, canvasScale);
	}
}