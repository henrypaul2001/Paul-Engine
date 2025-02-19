#pragma once
#include "RenderAPI.h"

namespace  PaulEngine {
	class RenderCommand {
	public:
		inline static void Init() {
			s_RenderAPI->Init();
		}

		inline static void SetViewport(const glm::vec2& position, const glm::vec2& size) {
			s_RenderAPI->SetViewport(position, size);
		}

		inline static void SetClearColour(const glm::vec4& colour) {
			s_RenderAPI->SetClearColour(colour);
		}

		inline static void Clear() {
			s_RenderAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			s_RenderAPI->DrawIndexed(vertexArray);
		}

	private:
		static RenderAPI* s_RenderAPI;
	};
}