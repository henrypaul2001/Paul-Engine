#include "UIImage.h"
namespace Engine {
	UIImage::UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture) : UIElement(position, scale, ResourceManager::GetInstance()->DefaultImageShader())
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
		shader->Use();

		shader->setVec2("translate", position);
		shader->setVec2("scale", scale);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, imageTexture->id);
		glBindVertexArray(ResourceManager::GetInstance()->GetUIQuadVAO());

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}