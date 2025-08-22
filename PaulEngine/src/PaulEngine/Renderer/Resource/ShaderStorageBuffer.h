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

	class ShaderStorageBuffer
	{
	public:
		virtual ~ShaderStorageBuffer() {}
	
		virtual void SetData(const void* data, size_t size, size_t offset = 0) = 0;
		virtual void Bind(uint32_t binding) = 0;

		static Ref<ShaderStorageBuffer> Create(size_t size, uint32_t binding, const StorageBufferMapping mapping = StorageBufferMapping::None, const bool dynamicStorage = true);
	};
}