#pragma once
#include "Buffer.h"

namespace PaulEngine
{
	enum class StorageBufferMapping
	{
		None = 0,

		MAP_READ,
		MAP_WRITE,
		MAP_READ_WRITE,

		MAP_READ_PERSISTENT,
		MAP_READ_COHERENT,
		
		MAP_WRITE_PERSISTENT,
		MAP_WRITE_COHERENT,

		MAP_READ_WRITE_PERSISTENT,
		MAP_READ_WRITE_COHERENT
	};

	constexpr bool IsStorageBufferMappingPersistent(const StorageBufferMapping mapping)
	{
		return (int)mapping >= 4;
	}
	constexpr bool IsStorageBufferMappingReadable(const StorageBufferMapping mapping)
	{
		return (mapping == StorageBufferMapping::MAP_READ ||
			mapping == StorageBufferMapping::MAP_READ_WRITE ||
			mapping == StorageBufferMapping::MAP_READ_PERSISTENT ||
			mapping == StorageBufferMapping::MAP_READ_COHERENT ||
			mapping == StorageBufferMapping::MAP_READ_WRITE_PERSISTENT ||
			mapping == StorageBufferMapping::MAP_READ_WRITE_COHERENT);
	}
	constexpr bool IsStorageBufferMappingWriteable(const StorageBufferMapping mapping)
	{
		return (mapping == StorageBufferMapping::MAP_WRITE ||
			mapping == StorageBufferMapping::MAP_READ_WRITE ||
			mapping == StorageBufferMapping::MAP_WRITE_PERSISTENT ||
			mapping == StorageBufferMapping::MAP_WRITE_COHERENT ||
			mapping == StorageBufferMapping::MAP_READ_WRITE_PERSISTENT ||
			mapping == StorageBufferMapping::MAP_READ_WRITE_COHERENT);
	}

	class ShaderStorageBuffer
	{
	public:
		virtual ~ShaderStorageBuffer() {}
	
		virtual void SetData(const void* data, size_t size, size_t offset = 0, const bool preferMap = true) = 0;
		virtual void ReadData(void* destination, size_t sourceSize, size_t sourceOffset = 0, const bool preferMap = true) = 0;
		virtual void Bind(uint32_t binding) = 0;
		virtual void Bind() = 0;

		static Ref<ShaderStorageBuffer> Create(size_t size, uint32_t binding, const StorageBufferMapping mapping = StorageBufferMapping::None, const bool dynamicStorage = true);
	};

	struct DrawElementsIndirectCommand
	{
		uint32_t Count = 0;
		uint32_t InstanceCount = 0;
		uint32_t FirstIndex = 0;
		 int32_t BaseVertex = 0;
		uint32_t BaseInstance = 0;
	};

	class DrawIndirectBuffer
	{
	public:
		virtual ~DrawIndirectBuffer() {}

		struct DrawIndirectSetDataParams
		{
			const DrawElementsIndirectCommand* data;
			uint32_t numCommands;
			uint32_t commandOffset;
		};
		static void GetDataRange(std::vector<DrawIndirectSetDataParams> multiDataParams, size_t& out_Start, size_t& out_End)
		{
			const size_t commandSize = sizeof(DrawElementsIndirectCommand);
			size_t dataStart = std::numeric_limits<size_t>::max();
			size_t dataEnd = 0;

			for (DrawIndirectSetDataParams& data : multiDataParams)
			{
				size_t currentStart = data.commandOffset * commandSize;
				if (currentStart < dataStart)
				{
					dataStart = currentStart;
				}

				size_t currentEnd = currentStart + (data.numCommands * commandSize);
				if (currentEnd > dataEnd)
				{
					dataEnd = currentEnd;
				}
			}

			out_Start = dataStart;
			out_End = dataEnd;
		}
		virtual void SetData(DrawIndirectSetDataParams dataParams, const bool preferMap = true) = 0;
		virtual void MultiSetData(std::vector<DrawIndirectSetDataParams> multiDataParams, const bool preferMap = true) = 0;
		virtual void ReadData(DrawElementsIndirectCommand* destination, uint32_t sourceNumCommands, uint32_t sourceCommandOffset = 0, const bool preferMap = true) = 0;
		virtual void Bind() = 0;

		static Ref<DrawIndirectBuffer> Create(uint32_t commandCount, const StorageBufferMapping mapping = StorageBufferMapping::None, const bool dynamicStorage = true);
	};
}