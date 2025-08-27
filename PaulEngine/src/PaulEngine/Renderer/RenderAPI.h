#pragma once
#include <glm/glm.hpp>
#include "Resource/VertexArray.h"
#include "RenderPipeline.h"

namespace PaulEngine {
	class RenderAPI {
	public:
		enum class API {
			None = 0,
			OpenGL = 1,
			Direct3D = 2, Vulkan = 3 // ambitious I know
		};

	public:
		virtual ~RenderAPI() {}
		virtual void Init() = 0;
		virtual void SetViewport(const glm::ivec2& position, const glm::ivec2& size) = 0;
		virtual void SetClearColour(const glm::vec4& colour) = 0;
		virtual void Clear(int bufferMask) = 0;
		virtual void ClearDepth() = 0;

		virtual void DrawIndexedBaseVertex(const uint32_t baseVertex, const uint32_t baseIndex, const uint32_t indexCount) = 0;
		virtual void DrawIndexedBaseVertex(const uint32_t baseVertex, const uint32_t baseIndex, const uint32_t indexCount, const DrawPrimitive drawPrimitive) = 0;

		virtual void MultiDrawIndexedIndirect(const uint32_t count, const uint32_t offset = 0, const size_t stride = 0) = 0;
		virtual void MultiDrawIndexedIndirect(const uint32_t count, const uint32_t offset, const size_t stride, const DrawPrimitive drawPrimitive) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount) = 0;
		
		virtual void SetLineWidth(float thickness) = 0;

		virtual void EnableDepth() = 0;
		virtual void DisableDepth() = 0;
		virtual void DepthMask(bool write) = 0;
		virtual void DepthFunc(DepthFunc func) = 0;
		virtual void SetFaceCulling(FaceCulling cullState) = 0;
		virtual void EnableBlend() = 0;
		virtual void DisableBlend() = 0;
		virtual void BlendFunc(BlendFunc srcFactor, BlendFunc dstFactor) = 0;
		virtual void BlendColour(glm::vec4 constantColour) = 0;
		virtual void BlendEquation(BlendEquation blendEquation) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}