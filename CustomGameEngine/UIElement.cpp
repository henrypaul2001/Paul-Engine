#include "UIElement.h"
#include "ResourceManager.h"
#include "RenderManager.h"
namespace Engine {
	UIElement::UIElement(glm::vec2 position, glm::vec2 scale, Shader* shader, UIBackground background)
	{
		this->position = position;
		this->scale = scale;
		this->shader = shader;
		isActive = true;
		this->background = background;
		useBackground = true;
	}

	UIElement::UIElement(glm::vec2 position, glm::vec2 scale, Shader* shader)
	{
		this->position = position;
		this->scale = scale;
		this->shader = shader;
		isActive = true;
		useBackground = false;
	}

	UIElement::~UIElement()
	{
	}

	void UIElement::Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale)
	{
		if (useBackground) {
			// Draw background
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);

			Shader* backgroundShader = ResourceManager::GetInstance()->DefaultUIBackgroundShader();
			backgroundShader->Use();

			glm::vec2 transformedPosition = position + canvasPosition;
			glm::vec4 extents = background.LeftRightUpDownExtents;
			extents.x *= canvasScale.x;
			extents.y *= canvasScale.x;
			extents.z *= canvasScale.y;
			extents.w *= canvasScale.y;
			
			RenderManager* renderManager = RenderManager::GetInstance();

			glm::mat4 projection = glm::ortho(0.0f, (float)renderManager->ScreenWidth(), 0.0f, (float)renderManager->ScreenHeight());

			backgroundShader->setVec2("translation", transformedPosition);
			backgroundShader->setVec4("LeftRightUpDownExtents", extents);
			backgroundShader->setVec4("Colour", background.Colour);
			backgroundShader->setMat4("projection", projection);

			glBindVertexArray(ResourceManager::GetInstance()->GetPointVAO());
			glDrawArrays(GL_POINTS, 0, 1);
			glBindVertexArray(0);
		}
	}
}