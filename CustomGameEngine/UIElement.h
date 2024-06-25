#pragma once
#include <glm/ext/vector_float2.hpp>
#include "Shader.h"
namespace Engine {
	enum UITypes {
		UI_TEXT,
		UI_IMAGE,
		UI_BUTTON,
	};

	struct UIBackground {
		glm::vec4 LeftRightUpDownExtents; // x = left, y = right, z = up, w = down

		float BorderThickness;

		bool Bordered;

		glm::vec4 Colour;
		glm::vec4 BorderColour;

		UIBackground() : LeftRightUpDownExtents(glm::vec4(0.0f)), BorderThickness(0.0f), Bordered(false), Colour(glm::vec4(1.0f)), BorderColour(glm::vec4(1.0f)) {}
	};

	class UIElement
	{
	public:
		UIElement(glm::vec2 position, glm::vec2 scale, Shader* shader, UIBackground background);
		UIElement(glm::vec2 position, glm::vec2 scale, Shader* shader);
		~UIElement();

		virtual void Draw(glm::vec2 canvasPosition, glm::vec2 canvasScale);

		void Position(glm::vec2 newPosition) { position = newPosition; }
		const glm::vec2& Position() const { return position; }

		void Scale(glm::vec2 newScale) { scale = newScale; }
		const glm::vec2& Scale() const { return scale; }

		void SetShader(Shader* newShader) { shader = newShader; }
		const Shader* GetShader() const { return shader; }

		void SetActive(bool active) { isActive = active; }
		const bool GetActive() const { return isActive; }

		const UITypes& UIType() const { return type; }

		const UIBackground& GetBackground() const { return background; }
		void SetBackground(const UIBackground newBackground) { background = newBackground; }

		const bool UseBackground() const { return useBackground; }
		void UseBackground(const bool useBackground) { this->useBackground = useBackground; }
	protected:
		UITypes type;

		Shader* shader;
		glm::vec2 position; // 0.0 - 1.0 XY
		glm::vec2 scale;

		bool isActive;

		UIBackground background;
		bool useBackground;
	};
}