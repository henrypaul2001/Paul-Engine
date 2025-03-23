#pragma once
#include <cstdint>
#include <cstring>
#include <malloc.h>

namespace PaulEngine
{
	// Non-owning
	struct Buffer
	{
		uint8_t* m_Data = nullptr;

		Buffer() = default;
		Buffer(uint64_t size)
		{
			Allocate(size);
		}
		Buffer(const void* data, uint64_t size) : m_Data((uint8_t*)data), m_Size(size) {}
		Buffer(const Buffer&) = default;

		uint64_t Size() { return m_Size; }

		static Buffer Copy(Buffer other)
		{
			Buffer result = Buffer(other.m_Size);
			memcpy(result.m_Data, other.m_Data, other.m_Size);
			return result;
		}

		void Allocate(uint64_t size)
		{
			Release();

			m_Data = (uint8_t*)malloc(size);
			m_Size = size;
		}

		void Release()
		{
			free(m_Data);
			m_Data = nullptr;
			m_Size = 0;
		}

		template <typename T>
		T* As()
		{
			return (T*)m_Data;
		}

		operator bool() const
		{
			return (bool)m_Data;
		}

	private:
		uint64_t m_Size = 0;
	};

	// Memory is released when ScopedBuffer goes out of scope
	struct ScopedBuffer
	{
		ScopedBuffer(Buffer buffer) : m_Buffer(buffer) {}
		ScopedBuffer(uint64_t size) : m_Buffer(size) {}
		~ScopedBuffer()
		{
			m_Buffer.Release();
		}

		uint8_t* Data() { return m_Buffer.m_Data; }
		uint64_t Size() { return m_Buffer.Size(); }

		template <typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer; }

	private:
		Buffer m_Buffer;
	};
}