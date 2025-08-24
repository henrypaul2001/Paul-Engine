#pragma once
#include "OpenGLShaderStorageBuffer.h"

namespace PaulEngine
{
	class OpenGLDrawIndirectBuffer : public DrawIndirectBuffer
	{
	public:
		OpenGLDrawIndirectBuffer(uint32_t commandCount, const StorageBufferMapping mapping = StorageBufferMapping::None, const bool dynamicStorage = true);
		virtual ~OpenGLDrawIndirectBuffer();

		virtual void SetData(const DrawElementsIndirectCommand* data, uint32_t numCommands, uint32_t commandOffset = 0, const bool preferMap = true) override;
		virtual void ReadData(DrawElementsIndirectCommand* destination, uint32_t sourceNumCommands, uint32_t sourceCommandOffset = 0, const bool preferMap = true) override;
		virtual void Bind() override;

	private:
		uint32_t m_RendererID;
		const StorageBufferMapping m_Mapping;
		const bool m_DynamicStorage;

		void BufferSubData(const DrawElementsIndirectCommand* data, size_t size, size_t offset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) before subsequent commands will see the updated data
		void SetMappedData(const DrawElementsIndirectCommand* data, size_t size, size_t offset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) before subsequent commands will see the updated data
		void SetMappedDataPersistent(const DrawElementsIndirectCommand* data, size_t size, size_t offset);

		void SetMappedDataCoherent(const DrawElementsIndirectCommand* data, size_t size, size_t offset);

		void GetBufferSubData(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) and then call glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE) for the CPU to see the GPU updated data
		void ReadMappedData(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset);

		// needs to call glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT) and then call glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE) for the CPU to see the GPU updated data
		void ReadMappedDataPersistent(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset);

		// needs to call glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE) for the CPU to see the GPU updated data
		void ReadMappedDataCoherent(DrawElementsIndirectCommand* destination, size_t sourceSize, size_t sourceOffset);

		void MapPersistent(size_t size, size_t offset);
		void ValidatePersistentMapping(size_t size, size_t offset);

		static std::unordered_map<int, void(OpenGLDrawIndirectBuffer::*)(const DrawElementsIndirectCommand*, size_t, size_t)> s_WriteFunctions;
		static std::unordered_map<int, void(OpenGLDrawIndirectBuffer::*)(DrawElementsIndirectCommand*, size_t, size_t)> s_ReadFunctions;

		GLvoid* m_Ptr;
		size_t m_CurrentMapSize;
		size_t m_CurrentMapOffset;
	};
}