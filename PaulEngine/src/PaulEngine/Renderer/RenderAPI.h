#pragma once
#include <glm/glm.hpp>
#include "VertexArray.h"

namespace PaulEngine {
	class RenderAPI {
	public:
		enum class API {
			None = 0,
			OpenGL = 1,
			Direct3D = 2, Vulkan = 3 // ambitious I know
		};

	public:
		virtual void SetViewport(const glm::vec2& position, const glm::vec2& size) = 0;
		virtual void SetClearColour(const glm::vec4& colour) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}