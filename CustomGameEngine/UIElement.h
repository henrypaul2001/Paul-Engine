#pragma once
#include <glm/ext/vector_float2.hpp>
namespace Engine {
	class UIElement
	{
	public:
		UIElement(glm::vec2 position);
		~UIElement();

		void Position(glm::vec2 newPosition) { position = newPosition; }
		const glm::vec2& Position() const { return position; }
	protected:
		glm::vec2 position; // 0.0 - 1.0 XY
	};
}