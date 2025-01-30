#pragma once
#include "UIButton.h"
#include "UIImage.h"
namespace Engine {
	class UIImageButton : public UIImage, public UIButton
	{
	public:
		UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, int idTag);
		UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, int idTag, UIBackground background);
		UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, Shader* shader, int idTag, UIBackground background);
		~UIImageButton();
	
		void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) const override;
	private:

	};
}