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

		virtual void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale) = 0;

		void Position(glm::vec2 newPosition) { position = newPosition; }
		const glm::vec2& Position() const { return position; }

		void Scale(glm::vec2 newScale) { scale = newScale; }
		const glm::vec2& Scale() const { return scale; }

		void SetShader(Shader* newShader) { shader = newShader; }
		const Shader* GetShader() const { return shader; }

		void SetActive(bool active) { isActive = active; }
		const bool GetActive() const { return isActive; }

		const UITypes& UIType() const { return type; }
	protected:
		UITypes type;

		Shader* shader;
		glm::vec2 position; // 0.0 - 1.0 XY
		glm::vec2 scale;

		bool isActive;
	};
}