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

	std::unordered_map<int, void(OpenGLShaderStorageBuffer::*)(std::vector<ShaderStorageBuffer::SetDataParams>)> OpenGLShaderStorageBuffer::s_WriteFunctions = {
		{ (int)StorageBufferMapping::MAP_WRITE, &MultiSetMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &MultiSetMappedData },
		{ (int)StorageBufferMapping::MAP_WRITE_PERSISTENT, &MultiSetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &MultiSetMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_WRITE_COHERENT, &MultiSetMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &MultiSetMappedDataCoherent }
	};
	std::unordered_map<int, void(OpenGLShaderStorageBuffer::*)(void*, size_t, size_t)> OpenGLShaderStorageBuffer::s_ReadFunctions = {
		{ (int)StorageBufferMapping::MAP_READ, &ReadMappedData },
		{ (int)StorageBufferMapping::MAP_READ_WRITE, &ReadMappedData },
		{ (int)StorageBufferMapping::MAP_READ_PERSISTENT, &ReadMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_PERSISTENT, &ReadMappedDataPersistent },
		{ (int)StorageBufferMapping::MAP_READ_COHERENT, &ReadMappedDataCoherent },
		{ (int)StorageBufferMapping::MAP_READ_WRITE_COHERENT, &ReadMappedDataCoherent }
	};

	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size, uint32_t binding, const StorageBufferMapping mapping, const bool dynamicStorage) : m_RendererID(0), m_Binding(binding), m_Mapping(mapping), m_DynamicStorage(dynamicStorage), m_Ptr(nullptr), m_CurrentMapSize(size), m_CurrentMapOffset(0)
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

	void OpenGLShaderStorageBuffer::SetData(SetDataParams dataParams, const bool preferMap)
	{
		MultiSetData({ dataParams }, preferMap);
	}

	void OpenGLShaderStorageBuffer::MultiSetData(std::vector<SetDataParams> multiDataParams, const bool preferMap)
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
		m_Binding = binding;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
	}

	void OpenGLShaderStorageBuffer::Bind()
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Binding, m_RendererID);
	}

	void OpenGLShaderStorageBuffer::BufferSubData(const void* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void OpenGLShaderStorageBuffer::MultiBufferSubData(std::vector<SetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();
		for (SetDataParams& data : multiDataParams)
		{
			glNamedBufferSubData(m_RendererID, data.offset, data.size, data.data);
		}
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

	void OpenGLShaderStorageBuffer::MultiSetMappedData(std::vector<SetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();

		// Prepare map
		size_t dataStart = 0;
		size_t dataEnd = 0;
		ShaderStorageBuffer::GetDataRange(multiDataParams, dataStart, dataEnd);

		PE_CORE_ASSERT(dataStart < dataEnd, "Invalid buffer map range");

		// Map full range of all data sets
		m_CurrentMapOffset = dataStart;
		m_CurrentMapSize = (dataEnd - dataStart);
		m_Ptr = glMapNamedBufferRange(m_RendererID, m_CurrentMapOffset, m_CurrentMapSize, GL_MAP_WRITE_BIT);

		// Individually copy each data pointer into mapped buffer
		if (m_Ptr)
		{
			for (SetDataParams& data : multiDataParams)
			{
				uint8_t* shiftedPtr = static_cast<uint8_t*>(m_Ptr) + (data.offset - m_CurrentMapOffset); // incremented in bytes

				memcpy((void*)shiftedPtr, data.data, data.size);
			}
			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
			glUnmapNamedBuffer(m_RendererID);
		}

		m_Ptr = nullptr;
		m_CurrentMapOffset = 0;
		m_CurrentMapSize = 0;
	}

	void OpenGLShaderStorageBuffer::SetMappedDataPersistent(const void* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		memcpy(m_Ptr, data, size);
		glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}

	void OpenGLShaderStorageBuffer::MultiSetMappedDataPersistent(std::vector<SetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();

		// Prepare map
		size_t dataStart = 0;
		size_t dataEnd = 0;
		ShaderStorageBuffer::GetDataRange(multiDataParams, dataStart, dataEnd);

		ValidatePersistentMapping(dataEnd - dataStart, dataStart);

		if (m_Ptr)
		{
			for (SetDataParams& data : multiDataParams)
			{
				uint8_t* shiftedPtr = static_cast<uint8_t*>(m_Ptr) + (data.offset - m_CurrentMapOffset); // incremented in bytes

				memcpy((void*)shiftedPtr, data.data, data.size);
			}

			glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
		}
	}

	void OpenGLShaderStorageBuffer::SetMappedDataCoherent(const void* data, size_t size, size_t offset)
	{
		PE_PROFILE_FUNCTION();
		ValidatePersistentMapping(size, offset);
		memcpy(m_Ptr, data, size);
	}

	void OpenGLShaderStorageBuffer::MultiSetMappedDataCoherent(std::vector<SetDataParams> multiDataParams)
	{
		PE_PROFILE_FUNCTION();

		// Prepare map
		size_t dataStart = 0;
		size_t dataEnd = 0;
		ShaderStorageBuffer::GetDataRange(multiDataParams, dataStart, dataEnd);

		ValidatePersistentMapping(dataEnd - dataStart, dataStart);

		if (m_Ptr)
		{
			for (SetDataParams& data : multiDataParams)
			{
				uint8_t* shiftedPtr = static_cast<uint8_t*>(m_Ptr) + (data.offset - m_CurrentMapOffset); // incremented in bytes

				memcpy((void*)shiftedPtr, data.data, data.size);
			}
		}
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