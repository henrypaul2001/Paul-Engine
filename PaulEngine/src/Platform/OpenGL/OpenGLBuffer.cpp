#include "pepch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace PaulEngine {

	constexpr GLenum OpenGLBufferUtils::BufferUsageToGLEnum(const BufferUsage usage)
	{
		switch (usage)
		{
			case BufferUsage::None: return 0;
		
			case BufferUsage::STATIC_DRAW: return GL_STATIC_DRAW;
			case BufferUsage::STATIC_READ: return GL_STATIC_READ;
			case BufferUsage::STATIC_COPY: return GL_STATIC_COPY;
		
			case BufferUsage::DYNAMIC_DRAW: return GL_DYNAMIC_DRAW;
			case BufferUsage::DYNAMIC_READ: return GL_DYNAMIC_READ;
			case BufferUsage::DYNAMIC_COPY: return GL_DYNAMIC_COPY;
		
			case BufferUsage::STREAM_DRAW: return GL_STREAM_DRAW;
			case BufferUsage::STREAM_READ: return GL_STREAM_READ;
			case BufferUsage::STREAM_COPY: return GL_STREAM_COPY;
		}
		PE_CORE_ERROR("Undefined BufferUsage translation");
		return 0;
	}

	// --   VertexBuffer   --
	// ----------------------

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, BufferUsage usage) : m_RendererID(0), m_Usage(usage)
	{
		PE_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, OpenGLBufferUtils::BufferUsageToGLEnum(usage));
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size, BufferUsage usage) : m_RendererID(0), m_Usage(usage)
	{
		PE_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, OpenGLBufferUtils::BufferUsageToGLEnum(usage));
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		PE_PROFILE_FUNCTION();
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
		//glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		//glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	// --   IndexBuffer   --
	// ---------------------

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count, BufferUsage usage) : m_RendererID(0), m_Count(count), m_Usage(usage)
	{
		PE_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), nullptr, OpenGLBufferUtils::BufferUsageToGLEnum(usage));
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count, BufferUsage usage) : m_RendererID(0), m_Count(count), m_Usage(usage)
	{
		PE_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, OpenGLBufferUtils::BufferUsageToGLEnum(usage));
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		PE_PROFILE_FUNCTION();
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::SetData(const uint32_t* indices, uint32_t count, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, count * sizeof(uint32_t), indices);
	}
}