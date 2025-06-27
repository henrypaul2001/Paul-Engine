#pragma once
#include "PaulEngine/Renderer/RenderAPI.h"

namespace PaulEngine {
	class OpenGLRenderAPI : public RenderAPI {
	public:
		OpenGLRenderAPI();
		~OpenGLRenderAPI();

		virtual void Init() override;
		virtual void SetViewport(const glm::ivec2& position, const glm::ivec2& size) override;
		virtual void SetClearColour(const glm::vec4& colour) override;
		virtual void Clear(int bufferMask) override;
		virtual void ClearDepth() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount) override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount) override;

		virtual void SetLineWidth(float thickness) override;

		static int BufferBitMaskToGLBitMask(int bufferMask);
	};
}