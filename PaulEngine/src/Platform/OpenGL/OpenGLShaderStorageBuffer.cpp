#include "pepch.h"
#include "OpenGLShaderStorageBuffer.h"

#include "OpenGLBuffer.h"
#include <glad/gl.h>

namespace PaulEngine
{
	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size, uint32_t binding, const StorageBufferMapping mapping, const bool dynamicStorage) : m_RendererID(0), m_Mapping(mapping), m_DynamicStorage(dynamicStorage), m_Ptr(nullptr), m_CurrentMapSize(size), m_CurrentMapOffset(0)
	{
		GLbitfield flags = 0;
		if (dynamicStorage) { flags |= GL_DYNAMIC_STORAGE_BIT; }
		if (mapping != StorageBufferMapping::None) { flags |= OpenGLBufferUtils::StorageBufferMappingToGLEnum(mapping); }
		PE_CORE_ASSERT(flags != 0, "Invalid flags");

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferStorage(m_RendererID, size, nullptr, flags);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);

		if (OpenGLBufferUtils::IsStorageBufferMappingPersistent(mapping))
		{
			MapPersisent(size, 0);
		}
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset)
	{
		
	}

	void OpenGLShaderStorageBuffer::Bind(uint32_t binding)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
	}

	void OpenGLShaderStorageBuffer::BufferSubData(const void* data, size_t size, size_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLShaderStorageBuffer::SetMappedData(const void* data, size_t size, size_t offset)
	{
		m_Ptr = glMapNamedBufferRange(m_RendererID, offset, size, GL_MAP_WRITE_BIT);
		if (m_Ptr)
		{
			memcpy(m_Ptr, data, size);
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		}
		glUnmapNamedBuffer(m_RendererID);
		m_Ptr = nullptr;
	}

	void OpenGLShaderStorageBuffer::SetMappedDataPersistent(const void* data, size_t size, size_t offset)
	{
		if (m_CurrentMapSize != size || m_CurrentMapOffset != offset)
		{
			// Remap
			MapPersisent(size, offset);
		}
		memcpy(m_Ptr, data, size);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}

	void OpenGLShaderStorageBuffer::SetMappedDataCoherent(const void* data, size_t size, size_t offset)
	{
		if (m_CurrentMapSize != size || m_CurrentMapOffset != offset)
		{
			// Remap
			MapPersisent(size, offset);
		}
		memcpy(m_Ptr, data, size);
	}

	void OpenGLShaderStorageBuffer::ReadMappedData()
	{

	}

	void OpenGLShaderStorageBuffer::ReadMappedDataPersistent()
	{

	}

	void OpenGLShaderStorageBuffer::ReadMappedDataCoherent()
	{

	}

	void OpenGLShaderStorageBuffer::MapPersisent(size_t size, size_t offset)
	{
		m_Ptr = glMapNamedBufferRange(m_RendererID, offset, size, OpenGLBufferUtils::StorageBufferMappingToGLEnum(m_Mapping));
		m_CurrentMapSize = size;
		m_CurrentMapOffset = offset;
	}
}