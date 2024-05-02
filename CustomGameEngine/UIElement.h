#pragma once
#include <glm/ext/vector_float2.hpp>
namespace Engine {
	class UIElement
	{
	public:
		UIElement(glm::vec2 position, glm::vec2 scale);
		~UIElement();

		void Position(glm::vec2 newPosition) { position = newPosition; }
		const glm::vec2& Position() const { return position; }

		void Scale(glm::vec2 newScale) { scale = newScale; }
		const glm::vec2& Scale() const { return scale; }

	protected:
		glm::vec2 position; // 0.0 - 1.0 XY
		glm::vec2 scale;
	};
}