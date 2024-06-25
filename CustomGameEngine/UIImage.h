#pragma once
#include "UIElement.h"
#include "Mesh.h"
#include "ResourceManager.h"
namespace Engine {
	class UIImage : virtual public UIElement
	{
	public:
		UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture);
		UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, UIBackground background);
		UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, UIBackground background, Shader* shader);
		~UIImage();

		void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) override;

		void SetTexture(Texture* newTexture) { imageTexture = newTexture; }
		const Texture* GetTexture() const { return imageTexture; }
	private:
		Texture* imageTexture;
	};
}