#include "pepch.h"
#include "OpenGLDrawIndirectBuffer.h"

#include "OpenGLBuffer.h"
#include <glad/gl.h>

namespace PaulEngine
{
	std::unordered_map<int, void(OpenGLDrawIndirectBuffer::*)(const DrawElementsIndirectCommand*, size_t, size_t)> OpenGLDrawIndirectBuffer::s_WriteFunctions = {
		{ (int)StorageBufferMapping::MAP_WRITE, &SetMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &SetMappedData },
		{ (int)StorageBufferMapping::MAP_WRITE_PERSISTENT, &SetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &SetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_WRITE_COHERENT, &SetMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &SetMappedDataCoherent }
	};
	std::unordered_map<int, void(OpenGLDrawIndirectBuffer::*)(DrawElementsIndirectCommand*, size_t, size_t)> OpenGLDrawIndirectBuffer::s_ReadFunctions = {
		{ (int)StorageBufferMapping::MAP_READ, &ReadMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &ReadMappedData },
		{ (int)StorageBufferMapping::MAP_READ_PERSISTENT, &ReadMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &ReadMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_COHERENT, &ReadMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &ReadMappedDataCoherent }
	};

	OpenGLDrawIndirectBuffer::OpenGLDrawIndirectBuffer(uint32_t commandCount, const StorageBufferMapping mapping, const bool dynamicStorage) : m_RendererID(0), m_Mapping(mapping), m_DynamicStorage(dynamicStorage), m_Ptr(nullptr), m_CurrentMapSize(commandCount * sizeof(DrawElementsIndirectCommand)), m_CurrentMapOffset(0)
	{
		PE_PROFILE_FUNCTION();
		GLbitfield flags = 0;
		if (dynamicStorage) { flags |= GL_DYNAMIC_STORAGE_BIT; }
		if (mapping != StorageBufferMapping::None) { flags |= OpenGLShaderStorageBufferUtils::StorageBufferMappingToGLEnum(mapping); }
		PE_CORE_ASSERT(flags != 0, "Invalid flags");

		size_t size = commandCount * sizeof(DrawElementsIndirectCommand);

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferStorage(m_RendererID, size, nullptr, flags);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID);

		if (IsStorageBufferMappingPersistent(m_Mapping))
		{
			MapPersistent(size, 0);
		}
	}

	OpenGLDrawIndirectBuffer::~OpenGLDrawIndirectBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLDrawIndirectBuffer::SetData(const DrawElementsIndirectCommand* data, uint32_t numCommands, uint32_t commandOffset, const bool preferMap)
	{
		PE_PROFILE_FUNCTION();
		const size_t commandSize = sizeof(DrawElementsIndirectCommand);
		const size_t size = numCommands * commandSize;
		const size_t offset = commandOffset * commandSize;
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

	void OpenGLDrawIndirectBuffer::ReadData(DrawElementsIndirectCommand* destination, uint32_t sourceNumCommands, uint32_t sourceCommandOffset, const bool preferMap)
	{
		PE_PROFILE_FUNCTION();
		const size_t commandSize = sizeof(DrawElementsIndirectCommand);
		const size_t sourceSize = sourceNumCommands * commandSize;
		const size_t sourceOffset = sourceCommandOffset * commandSize;
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

	void OpenGLDrawIndirectBuffer::Bind()
	{
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID);
	}

	void OpenGLDrawIndirectBuffer::BufferSubData(const DrawElementsIndirectCommand* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLDrawIndirectBuffer::SetMappedData(const DrawElementsIndirectCommand* data, size_t size, size_t offset)
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

	void OpenGLDrawIndirectBuffer::SetMappedDataPersistent(const DrawElementsIndirectCommand* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		memcpy(m_Ptr, data, size);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}

	void OpenGLDrawIndirectBuffer::SetMappedDataCoherent(const DrawElementsIndirectCommand* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		memcpy(m_Ptr, data, size);
	}

	void OpenGLDrawIndirectBuffer::GetBufferSubData(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		glGetNamedBufferSubData(m_RendererID, sourceOffset, sourceSize, destination);
	}

	void OpenGLDrawIndirectBuffer::ReadMappedData(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset)
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

	void OpenGLDrawIndirectBuffer::ReadMappedDataPersistent(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(sourceSize, sourceOffset);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		memcpy(destination, m_Ptr, sourceSize);
		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
	}

	void OpenGLDrawIndirectBuffer::ReadMappedDataCoherent(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(sourceSize, sourceOffset);
		memcpy(destination, m_Ptr, sourceSize);
		GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
	}

	void OpenGLDrawIndirectBuffer::MapPersistent(size_t size, size_t offset)
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

	void OpenGLDrawIndirectBuffer::ValidatePersistentMapping(size_t size, size_t offset)
	{
		if (m_CurrentMapSize != size || m_CurrentMapOffset != offset)
		{
			// Remap
			MapPersistent(size, offset);
		}
	}
}