#include "UIImage.h"
namespace Engine {
	UIImage::UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture) : UIElement(position, scale, ResourceManager::GetInstance()->DefaultTextShader())
	{
		this->imageTexture = imageTexture;
	}

	UIImage::UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, Shader* shader) : UIElement(position, scale, shader)
	{
		this->imageTexture = imageTexture;
	}

	UIImage::~UIImage()
	{

	}

	void UIImage::Draw()
	{

	}
}