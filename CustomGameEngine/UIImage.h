#pragma once
#include "UIElement.h"
#include "Mesh.h"
#include "ResourceManager.h"
namespace Engine {
	class UIImage : public UIElement
	{
	public:
		UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture);
		UIImage(glm::vec2 position, glm::vec2 scale, Texture* imageTexture, Shader* shader);
		~UIImage();

		void Draw() override;

		void SetTexture(Texture* newTexture) { imageTexture = newTexture; }
		const Texture* GetTexture() const { return imageTexture; }
	private:
		Texture* imageTexture;
	};
}