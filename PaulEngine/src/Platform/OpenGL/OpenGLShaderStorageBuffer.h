#pragma once
#include "PaulEngine/Renderer/Resource/ShaderStorageBuffer.h"

typedef void GLvoid;
typedef unsigned int GLbitfield;

namespace PaulEngine
{
	namespace OpenGLShaderStorageBufferUtils
	{
		constexpr GLbitfield StorageBufferMappingToGLEnum(const StorageBufferMapping mapping);
	}

	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer(size_t size, uint32_t binding, const StorageBufferMapping mapping = StorageBufferMapping::None, const bool dynamicStorage = true);
		virtual ~OpenGLShaderStorageBuffer();

		virtual void SetData(const void* data, size_t size, size_t offset = 0, const bool preferMap = true) override;
		virtual void ReadData(void* destination, size_t sourceSize, size_t sourceOffset = 0, const bool preferMap = true) override;
		virtual void Bind(uint32_t binding) override;
		virtual void Bind() override;

	protected:
		uint32_t m_RendererID;
		uint32_t m_Binding;
		const StorageBufferMapping m_Mapping;
		const bool m_DynamicStorage;

		void BufferSubData(const void* data, size_t size, size_t offset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) before subsequent commands will see the updated data
		void SetMappedData(const void* data, size_t size, size_t offset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) before subsequent commands will see the updated data
		void SetMappedDataPersistent(const void* data, size_t size, size_t offset);

		void SetMappedDataCoherent(const void* data, size_t size, size_t offset);

		void GetBufferSubData(void* destination, size_t sourceSize, size_t sourceOffset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) and then call glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE) for the CPU to see the GPU updated data
		void ReadMappedData(void* destination, size_t sourceSize, size_t sourceOffset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) and then call glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE) for the CPU to see the GPU updated data
		void ReadMappedDataPersistent(void* destination, size_t sourceSize, size_t sourceOffset);

		// needs to call glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE) for the CPU to see the GPU updated data
		void ReadMappedDataCoherent(void* destination, size_t sourceSize, size_t sourceOffset);

		void MapPersistent(size_t size, size_t offset);
		void ValidatePersistentMapping(size_t size, size_t offset);

		static std::unordered_map<int, void(OpenGLShaderStorageBuffer::*)(const void*, size_t, size_t)> s_WriteFunctions;
		static std::unordered_map<int, void(OpenGLShaderStorageBuffer::*)(void*, size_t, size_t)> s_ReadFunctions;

		GLvoid* m_Ptr;
		size_t m_CurrentMapSize;
		size_t m_CurrentMapOffset;
	};
}