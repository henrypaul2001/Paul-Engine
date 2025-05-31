#pragma once
#include "PaulEngine/Renderer/Resource/Buffer.h"
#include <cstdint>

typedef unsigned int GLenum;

namespace PaulEngine {
	namespace OpenGLBufferUtils
	{
		constexpr GLenum BufferUsageToGLEnum(const BufferUsage usage);
	}

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, BufferUsage usage = BufferUsage::DYNAMIC_DRAW);
		OpenGLVertexBuffer(float* vertices, uint32_t size, BufferUsage usage = BufferUsage::STATIC_DRAW);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		virtual void SetData(const void* data, uint32_t size) override;

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
		BufferUsage m_Usage;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count, BufferUsage usage = BufferUsage::STATIC_DRAW);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetCount() override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
		BufferUsage m_Usage;
	};
}