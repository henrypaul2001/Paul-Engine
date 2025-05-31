#include "pepch.h"
#include "OpenGLUniformBuffer.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace PaulEngine
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding, BufferUsage usage) : m_RendererID(0), m_Usage(usage)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, OpenGLBufferUtils::BufferUsageToGLEnum(m_Usage));
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLUniformBuffer::Bind(uint32_t binding)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}
}