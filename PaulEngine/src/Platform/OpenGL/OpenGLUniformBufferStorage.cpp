#include "pepch.h"
#include "OpenGLUniformBufferStorage.h"
#include "OpenGLBuffer.h"

#include <glad/gl.h>

namespace PaulEngine
{
	OpenGLUniformBufferStorage::OpenGLUniformBufferStorage(std::vector<BufferElement> layout, uint32_t binding, BufferUsage usage) : UniformBufferStorage(layout), m_Binding(binding), m_RendererID(0), m_Usage(usage)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, Size(), nullptr, OpenGLBufferUtils::BufferUsageToGLEnum(m_Usage));
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBufferStorage::~OpenGLUniformBufferStorage()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBufferStorage::UploadStorage()
	{
		if (m_IsDirty)
		{
			UploadStorageForced();
		}
	}

	void OpenGLUniformBufferStorage::UploadStorageForced()
	{
		PE_PROFILE_FUNCTION();
		glNamedBufferData(m_RendererID, Size(), (const void*)m_Buffer.Data(), OpenGLBufferUtils::BufferUsageToGLEnum(m_Usage));
		m_IsDirty = false;
	}

	void OpenGLUniformBufferStorage::Bind(uint32_t binding)
	{
		m_Binding = binding;
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	void OpenGLUniformBufferStorage::Bind()
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_RendererID);
	}
}