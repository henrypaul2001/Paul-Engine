#pragma once
#include "UIButton.h"
#include "UIImage.h"
namespace Engine {
	class UIImageButton : public UIImage, public UIButton
	{
	public:
		UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture);
		UIImageButton(glm::vec2 position, glm::vec2 imageScale, glm::vec2 buttonScale, Texture* imageTexture, Shader* shader);
		~UIImageButton();
	
		void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) override;
	private:

	};
}