#include "UIImageButton.h"
namespace Engine {
	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, int idTag) : UIElement(position, imageScale, ResourceManager::GetInstance()->DefaultImageShader()), UIImage(position, imageScale, imageTexture), UIButton(position, buttonScale, ResourceManager::GetInstance()->DefaultImageShader(), idTag)
	{
		buttonType = BUTTON_IMAGE;
	}

	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, int idTag, UIBackground background) : UIElement(position, imageScale, ResourceManager::GetInstance()->DefaultImageShader(), background), UIImage(position, imageScale, imageTexture, background), UIButton(position, buttonScale, ResourceManager::GetInstance()->DefaultImageShader(), idTag, background)
	{
		buttonType = BUTTON_IMAGE;
	}

	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, Shader* shader, int idTag, UIBackground background) : UIElement(position, imageScale, shader, background), UIImage(position, imageScale, imageTexture, background, shader), UIButton(position, buttonScale, shader, idTag, background)
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