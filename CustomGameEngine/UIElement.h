#pragma once
#include <glm/ext/vector_float2.hpp>
#include "Shader.h"
namespace Engine {
	enum UITypes {
		UI_TEXT,
		UI_IMAGE,
		UI_BUTTON,
	};

	class UIElement
	{
	public:
		UIElement(glm::vec2 position, glm::vec2 scale, Shader* shader);
		~UIElement();

		virtual void Draw() = 0;

		void Position(glm::vec2 newPosition) { position = newPosition; }
		const glm::vec2& Position() const { return position; }

		void Scale(glm::vec2 newScale) { scale = newScale; }
		const glm::vec2& Scale() const { return scale; }

		void SetShader(Shader* newShader) { shader = newShader; }
		const Shader* GetShader() const { return shader; }

		const UITypes& UIType() const { return type; }
	protected:
		UITypes type;

		Shader* shader;
		glm::vec2 position; // 0.0 - 1.0 XY
		glm::vec2 scale;
	};
}