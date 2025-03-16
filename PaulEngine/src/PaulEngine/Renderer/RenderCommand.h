#pragma once
#include "RenderAPI.h"

namespace  PaulEngine {
	class RenderCommand {
	public:
		inline static void Init() {
			s_RenderAPI->Init();
		}

		inline static void SetViewport(const glm::ivec2& position, const glm::ivec2& size) {
			s_RenderAPI->SetViewport(position, size);
		}

		inline static void SetClearColour(const glm::vec4& colour) {
			s_RenderAPI->SetClearColour(colour);
		}

		inline static void Clear() {
			s_RenderAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			vertexArray->Bind();
			s_RenderAPI->DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount());
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount) {
			vertexArray->Bind();
			s_RenderAPI->DrawIndexed(vertexArray, indexCount);
		}

	private:
		static RenderAPI* s_RenderAPI;
	};
}