#pragma once
#include "PaulEngine/Renderer/RenderAPI.h"

namespace PaulEngine {
	class OpenGLRenderAPI : public RenderAPI {
	public:
		OpenGLRenderAPI();
		~OpenGLRenderAPI();

		virtual void SetViewport(const glm::vec2& position, const glm::vec2& size) override;
		virtual void SetClearColour(const glm::vec4& colour) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}