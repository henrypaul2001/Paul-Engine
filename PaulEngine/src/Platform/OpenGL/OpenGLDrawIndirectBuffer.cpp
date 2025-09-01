#include "pepch.h"
#include "OpenGLDrawIndirectBuffer.h"

#include "OpenGLBuffer.h"
#include <glad/gl.h>

namespace PaulEngine
{
	std::unordered_map<int, void(OpenGLDrawIndirectBuffer::*)(std::vector<DrawIndirectBuffer::DrawIndirectSetDataParams>)> OpenGLDrawIndirectBuffer::s_WriteFunctions = {
		{ (int)StorageBufferMapping::MAP_WRITE, &MultiSetMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &MultiSetMappedData },
		{ (int)StorageBufferMapping::MAP_WRITE_PERSISTENT, &MultiSetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &MultiSetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_WRITE_COHERENT, &MultiSetMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &MultiSetMappedDataCoherent }
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

	void OpenGLDrawIndirectBuffer::SetData(DrawIndirectSetDataParams dataParams, const bool preferMap)
	{
		MultiSetData({ dataParams }, preferMap);
	}

	void OpenGLDrawIndirectBuffer::MultiSetData(std::vector<DrawIndirectSetDataParams> multiDataParams, const bool preferMap)
	{
		PE_PROFILE_FUNCTION();
		if (preferMap && IsStorageBufferMappingWriteable(m_Mapping))
		{
			auto funcIt = s_WriteFunctions.find((int)m_Mapping);
			PE_CORE_ASSERT(funcIt != s_WriteFunctions.end(), "Undefined write function mapping");
			(this->*(funcIt->second))(multiDataParams);
		}
		else if (m_DynamicStorage)
		{
			MultiBufferSubData(multiDataParams);
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

	void OpenGLDrawIndirectBuffer::MultiBufferSubData(std::vector<DrawIndirectSetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();
		for (DrawIndirectSetDataParams& data : multiDataParams)
		{
			const size_t commandSize = sizeof(DrawElementsIndirectCommand);
			const size_t sourceSize = data.numCommands * commandSize;
			const size_t sourceOffset = data.commandOffset * commandSize;
			glNamedBufferSubData(m_RendererID, sourceOffset, sourceSize, data.data);
		}
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

	void OpenGLDrawIndirectBuffer::MultiSetMappedData(std::vector<DrawIndirectSetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();
		const size_t commandSize = sizeof(DrawElementsIndirectCommand);

		// Prepare map
		size_t dataStart = 0;
		size_t dataEnd = 0;
		DrawIndirectBuffer::GetDataRange(multiDataParams, dataStart, dataEnd);

		PE_CORE_ASSERT(dataStart < dataEnd, "Invalid buffer map range");

		// Map full range of all data sets
		m_CurrentMapOffset = dataStart;
		m_CurrentMapSize = (dataEnd - dataStart);
		m_Ptr = glMapNamedBufferRange(m_RendererID, m_CurrentMapOffset, m_CurrentMapSize, GL_MAP_WRITE_BIT);

		// Individually copy each data pointer into mapped buffer
		if (m_Ptr)
		{
			for (DrawIndirectSetDataParams& data : multiDataParams)
			{
				const size_t sourceSize = data.numCommands * commandSize;
				const size_t sourceOffset = data.commandOffset * commandSize;

				uint8_t* shiftedPtr = static_cast<uint8_t*>(m_Ptr) + (sourceOffset - m_CurrentMapOffset); // incremented in bytes

				memcpy((void*)shiftedPtr, data.data, sourceSize);
			}
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
			glUnmapNamedBuffer(m_RendererID);
		}

		m_Ptr = nullptr;
		m_CurrentMapOffset = 0;
		m_CurrentMapSize = 0;
	}

	void OpenGLDrawIndirectBuffer::SetMappedDataPersistent(const DrawElementsIndirectCommand* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		if (m_Ptr)
		{
			memcpy(m_Ptr, data, size);
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		}
	}

	void OpenGLDrawIndirectBuffer::MultiSetMappedDataPersistent(std::vector<DrawIndirectSetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();

		// Prepare map
		size_t dataStart = 0;
		size_t dataEnd = 0;
		DrawIndirectBuffer::GetDataRange(multiDataParams, dataStart, dataEnd);

		ValidatePersistentMapping(dataEnd - dataStart, dataStart);

		if (m_Ptr)
		{
			const size_t commandSize = sizeof(DrawElementsIndirectCommand);
			for (DrawIndirectSetDataParams& data : multiDataParams)
			{
				const size_t sourceSize = data.numCommands * commandSize;
				const size_t sourceOffset = data.commandOffset * commandSize;

				uint8_t* shiftedPtr = static_cast<uint8_t*>(m_Ptr) + (sourceOffset - m_CurrentMapOffset); // incremented in bytes

				memcpy((void*)shiftedPtr, data.data, sourceSize);
			}

			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		}
	}

	void OpenGLDrawIndirectBuffer::SetMappedDataCoherent(const DrawElementsIndirectCommand* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		if (m_Ptr)
		{
			memcpy(m_Ptr, data, size);
		}
	}

	void OpenGLDrawIndirectBuffer::MultiSetMappedDataCoherent(std::vector<DrawIndirectSetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();

		// Prepare map
		size_t dataStart = 0;
		size_t dataEnd = 0;
		DrawIndirectBuffer::GetDataRange(multiDataParams, dataStart, dataEnd);

		ValidatePersistentMapping(dataEnd - dataStart, dataStart);

		if (m_Ptr)
		{
			const size_t commandSize = sizeof(DrawElementsIndirectCommand);
			for (DrawIndirectSetDataParams& data : multiDataParams)
			{
				const size_t sourceSize = data.numCommands * commandSize;
				const size_t sourceOffset = data.commandOffset * commandSize;

				uint8_t* shiftedPtr = static_cast<uint8_t*>(m_Ptr) + (sourceOffset - m_CurrentMapOffset); // incremented in bytes

				memcpy((void*)shiftedPtr, data.data, sourceSize);
			}
		}
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