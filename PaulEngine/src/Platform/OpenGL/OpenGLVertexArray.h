#pragma once
#include "PaulEngine/Renderer/Resource/VertexArray.h"

namespace PaulEngine {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray(DrawPrimitive drawPrimitive = DrawPrimitive::TRIANGLES);
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		virtual DrawPrimitive GetDrawPrimitive() const override { return m_Primitive; }

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
		DrawPrimitive m_Primitive;
	};
}
