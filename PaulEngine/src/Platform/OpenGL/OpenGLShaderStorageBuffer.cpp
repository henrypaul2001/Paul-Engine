#include "pepch.h"
#include "OpenGLShaderStorageBuffer.h"

#include "OpenGLBuffer.h"
#include <glad/gl.h>

namespace PaulEngine
{
	constexpr GLbitfield OpenGLShaderStorageBufferUtils::StorageBufferMappingToGLEnum(const StorageBufferMapping mapping)
	{
		switch (mapping)
		{
		case StorageBufferMapping::None: return 0;

		case StorageBufferMapping::MAP_READ: return GL_MAP_READ_BIT;
		case StorageBufferMapping::MAP_WRITE: return GL_MAP_WRITE_BIT;
		case StorageBufferMapping::MAP_READ_WRITE: return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

		case StorageBufferMapping::MAP_READ_PERSISTENT: return GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT;
		case StorageBufferMapping::MAP_READ_COHERENT: return GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		case StorageBufferMapping::MAP_WRITE_PERSISTENT: return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
		case StorageBufferMapping::MAP_WRITE_COHERENT: return GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		case StorageBufferMapping::MAP_READ_WRITE_PERSISTENT: return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
		case StorageBufferMapping::MAP_READ_WRITE_COHERENT: return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		}
		PE_CORE_ERROR("Undefined StorageBufferMapping translation");
		return 0;
	}

	std::unordered_map<int, void(OpenGLShaderStorageBuffer::*)(const void*, size_t, size_t)> OpenGLShaderStorageBuffer::s_WriteFunctions = {
		{ (int)StorageBufferMapping::MAP_WRITE, &SetMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &SetMappedData },
		{ (int)StorageBufferMapping::MAP_WRITE_PERSISTENT, &SetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &SetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_WRITE_COHERENT, &SetMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &SetMappedDataCoherent }
	};
	std::unordered_map<int, void(OpenGLShaderStorageBuffer::*)(void*, size_t, size_t)> OpenGLShaderStorageBuffer::s_ReadFunctions = {
		{ (int)StorageBufferMapping::MAP_READ, &ReadMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &ReadMappedData },
		{ (int)StorageBufferMapping::MAP_READ_PERSISTENT, &ReadMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &ReadMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_COHERENT, &ReadMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &ReadMappedDataCoherent }
	};

	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size, uint32_t binding, const StorageBufferMapping mapping, const bool dynamicStorage) : m_RendererID(0), m_Mapping(mapping), m_DynamicStorage(dynamicStorage), m_Ptr(nullptr), m_CurrentMapSize(size), m_CurrentMapOffset(0)
	{
		PE_PROFILE_FUNCTION();
		GLbitfield flags = 0;
		if (dynamicStorage) { flags |= GL_DYNAMIC_STORAGE_BIT; }
		if (mapping != StorageBufferMapping::None) { flags |= OpenGLShaderStorageBufferUtils::StorageBufferMappingToGLEnum(mapping); }
		PE_CORE_ASSERT(flags != 0, "Invalid flags");

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferStorage(m_RendererID, size, nullptr, flags);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);

		if (IsStorageBufferMappingPersistent(m_Mapping))
		{
			MapPersistent(size, 0);
		}
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset, const bool preferMap)
	{
		PE_PROFILE_FUNCTION();
		if (preferMap && IsStorageBufferMappingWriteable(m_Mapping))
		{
			auto funcIt = s_WriteFunctions.find((int)m_Mapping);
			PE_CORE_ASSERT(funcIt != s_WriteFunctions.end(), "Undefined write function mapping");
			(this->*(funcIt->second))(data, size, offset);
		}
		else if (m_DynamicStorage)
		{
			BufferSubData(data, size, offset);
		}
		else
		{
			PE_CORE_ERROR("Attempting to modify immutable shader storage buffer");
		}
	}

	void OpenGLShaderStorageBuffer::ReadData(void* destination, size_t sourceSize, size_t sourceOffset, const bool preferMap)
	{
		PE_PROFILE_FUNCTION();
		if (preferMap && IsStorageBufferMappingReadable(m_Mapping))
		{
			auto funcIt = s_ReadFunctions.find((int)m_Mapping);
			PE_CORE_ASSERT(funcIt != s_ReadFunctions.end(), "Undefined read function mapping");
			(this->*(funcIt->second))(destination, sourceSize, sourceOffset);
		}
		else
		{
			GetBufferSubData(destination, sourceSize, sourceOffset);
		}
	}

	void OpenGLShaderStorageBuffer::Bind(uint32_t binding)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
	}

	void OpenGLShaderStorageBuffer::BufferSubData(const void* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLShaderStorageBuffer::SetMappedData(const void* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
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
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		memcpy(m_Ptr, data, size);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}

	void OpenGLShaderStorageBuffer::SetMappedDataCoherent(const void* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		memcpy(m_Ptr, data, size);
	}

	void OpenGLShaderStorageBuffer::GetBufferSubData(void* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		glGetNamedBufferSubData(m_RendererID, sourceOffset, sourceSize, destination);
	}

	void OpenGLShaderStorageBuffer::ReadMappedData(void* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		m_Ptr = glMapNamedBufferRange(m_RendererID, sourceOffset, sourceSize, GL_MAP_READ_BIT);
		if (m_Ptr)
		{
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
			memcpy(destination, m_Ptr, sourceSize);
			GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
		}
		glUnmapNamedBuffer(m_RendererID);
		m_Ptr = nullptr;
	}

	void OpenGLShaderStorageBuffer::ReadMappedDataPersistent(void* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(sourceSize, sourceOffset);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		memcpy(destination, m_Ptr, sourceSize);
		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
	}

	void OpenGLShaderStorageBuffer::ReadMappedDataCoherent(void* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(sourceSize, sourceOffset);
		memcpy(destination, m_Ptr, sourceSize);
		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
	}

	void OpenGLShaderStorageBuffer::MapPersistent(size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		if (m_Ptr) {
			glUnmapNamedBuffer(m_RendererID);
			m_Ptr = nullptr;
		}
		m_Ptr = glMapNamedBufferRange(m_RendererID, offset, size, OpenGLShaderStorageBufferUtils::StorageBufferMappingToGLEnum(m_Mapping));
		m_CurrentMapSize = size;
		m_CurrentMapOffset = offset;
	}

	void OpenGLShaderStorageBuffer::ValidatePersistentMapping(size_t size, size_t offset)
	{
		if (m_CurrentMapSize != size || m_CurrentMapOffset != offset)
		{
			// Remap
			MapPersistent(size, offset);
		}
	}
}