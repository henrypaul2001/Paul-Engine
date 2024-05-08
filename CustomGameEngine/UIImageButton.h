#pragma once
#include "UIButton.h"
#include "UIImage.h"
namespace Engine {
	class UIImageButton : public UIImage, public UIButton
	{
	public:
		UIImageButton(glm::vec2 position, glm::vec2 scale, Texture* imageTexture);
		UIImageButton(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, Shader* shader);
		~UIImageButton();
	
		void Draw() override;
	private:

	};
}