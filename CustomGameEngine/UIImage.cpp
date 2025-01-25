#include "UIImage.h"
namespace Engine {
	UIImage::UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture) : UIElement(position, scale, ResourceManager::GetInstance()->DefaultImageShader())
	{
		this->imageTexture = imageTexture;
		type = UI_IMAGE;
	}

	UIImage::UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, UIBackground background) : UIElement(position, scale, ResourceManager::GetInstance()->DefaultImageShader(), background)
	{
		this->imageTexture = imageTexture;
		type = UI_IMAGE;
	}

	UIImage::UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, UIBackground background, Shader* shader) : UIElement(position, scale, shader, background)
	{
		this->imageTexture = imageTexture;
		type = UI_IMAGE;
	}

	UIImage::~UIImage()
	{

	}

	void UIImage::Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) const
	{
		UIElement::Draw(canvasPosition, canvasScale);

		shader->Use();

		shader->setVec2("translate", position + canvasPosition);
		shader->setVec2("scale", scale * canvasScale);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, imageTexture->id);
		glBindVertexArray(ResourceManager::GetInstance()->GetUIQuadVAO());

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}