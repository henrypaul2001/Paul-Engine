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

		virtual void DrawIndexedBaseVertex(const uint32_t baseVertex, const uint32_t baseIndex, const uint32_t indexCount) override;
		virtual void DrawIndexedBaseVertex(const uint32_t baseVertex, const uint32_t baseIndex, const uint32_t indexCount, const DrawPrimitive drawPrimitive) override;

		virtual void MultiDrawIndexedIndirect(const uint32_t count, const uint32_t offset = 0, const size_t stride = 0) override;
		virtual void MultiDrawIndexedIndirect(const uint32_t count, const uint32_t offset, const size_t stride, const DrawPrimitive drawPrimitive) override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount) override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount) override;

		virtual void SetLineWidth(float thickness) override;

		virtual void EnableDepth() override;
		virtual void DisableDepth() override;
		virtual void DepthMask(bool write) override;
		virtual void DepthFunc(PaulEngine::DepthFunc func) override;
		virtual void SetFaceCulling(FaceCulling cullState) override;
		virtual void EnableBlend() override;
		virtual void DisableBlend() override;
		virtual void BlendFunc(PaulEngine::BlendFunc srcFactor, PaulEngine::BlendFunc dstFactor) override;
		virtual void BlendColour(glm::vec4 constantColour) override;
		virtual void BlendEquation(PaulEngine::BlendEquation blendEquation) override;

		static int BufferBitMaskToGLBitMask(int bufferMask);
	};
}