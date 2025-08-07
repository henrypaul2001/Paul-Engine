#pragma once
#include "RenderAPI.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"

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

		inline static void Clear(int bufferMask = (Framebuffer::BufferBit::COLOUR | Framebuffer::BufferBit::DEPTH | Framebuffer::BufferBit::STENCIL)) {
			s_RenderAPI->Clear(bufferMask);
		}

		inline static void ClearDepth() {
			s_RenderAPI->ClearDepth();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			s_RenderAPI->DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount());
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount) {
			s_RenderAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount) {
			s_RenderAPI->DrawLines(vertexArray, vertexCount);
		}

		inline static void SetLineWidth(const float thickness) {
			s_RenderAPI->SetLineWidth(thickness);
		}

		inline static void EnableDepth() {
			s_RenderAPI->EnableDepth();
		}

		inline static void DisableDepth() {
			s_RenderAPI->DisableDepth();
		}

		inline static void DepthMask(bool write) {
			s_RenderAPI->DepthMask(write);
		}

		inline static void DepthFunc(DepthFunc func) {
			s_RenderAPI->DepthFunc(func);
		}

		inline static void SetFaceCulling(FaceCulling cullState) {
			s_RenderAPI->SetFaceCulling(cullState);
		}

		inline static void EnableBlend() {
			s_RenderAPI->EnableBlend();
		}

		inline static void DisableBlend() {
			s_RenderAPI->DisableBlend();
		}

		inline static void BlendFunc(BlendFunc srcFactor, BlendFunc dstFactor) {
			s_RenderAPI->BlendFunc(srcFactor, dstFactor);
		}

		inline static void BlendColour(glm::vec4 constantColour) {
			s_RenderAPI->BlendColour(constantColour);
		}

		inline static void BlendEquation(BlendEquation blendEquation) {
			s_RenderAPI->BlendEquation(blendEquation);
		}

	private:
		static RenderAPI* s_RenderAPI;
	};
}