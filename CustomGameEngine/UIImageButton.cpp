#include "UIImageButton.h"
namespace Engine {
	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 scale, Texture* imageTexture) : UIElement(position, scale, ResourceManager::GetInstance()->DefaultImageShader()), UIImage(position, scale, imageTexture), UIButton(position, scale, ResourceManager::GetInstance()->DefaultImageShader())
	{
		buttonType = BUTTON_IMAGE;
	}

	UIImageButton::UIImageButton(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, Shader* shader) : UIElement(position, scale, shader), UIImage(position, scale, imageTexture, shader), UIButton(position, scale, shader)
	{
		buttonType = BUTTON_IMAGE;
	}

	UIImageButton::~UIImageButton()
	{
	}

	void UIImageButton::Draw()
	{
		UIImage::Draw();
	}
}