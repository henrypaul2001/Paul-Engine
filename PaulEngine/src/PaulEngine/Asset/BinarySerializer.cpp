#include "pepch.h"
#include "BinarySerializer.h"

#include <zlib.h>

namespace PaulEngine
{
	Buffer BinarySerializer::CompressBuffer(const Buffer buffer)
	{
		size_t srcLength = buffer.Size();
		size_t dstLength = buffer.Size() + 2; // 2 byte header

		const uint8_t* src = buffer.m_Data;
		uint8_t* dst = (uint8_t*)malloc(dstLength);

		int result = compress((Bytef*)dst, (uLongf*)&dstLength, (const Bytef*)src, (uLongf)srcLength);
		if (result != Z_OK)
		{
			PE_CORE_ERROR("Error compressing buffer: zlib '{0}'", result);
			free(dst);
			return Buffer();
		}

		void* compressedDst = realloc((void*)dst, dstLength);
		if (!compressedDst)
		{
			PE_CORE_ERROR("Error compressing buffer: realloc() fail");
			free(dst);
			return Buffer();
		}

		dst = nullptr;

		return Buffer(compressedDst, dstLength);
	}

	Buffer BinarySerializer::UncompressBuffer(const Buffer buffer, const size_t expectedSize)
	{
		size_t srcLength = buffer.Size();
		size_t dstLength = expectedSize;

		const uint8_t* src = buffer.m_Data;
		uint8_t* dst = (uint8_t*)malloc(expectedSize);

		int result = uncompress((Bytef*)dst, (uLongf*)&dstLength, (const Bytef*)src, (uLongf)srcLength);
		if (result != Z_OK)
		{
			PE_CORE_ERROR("Error uncompressing buffer: zlib '{0}'", result);
			free(dst);
			return Buffer();
		}

		void* uncompressedDst = realloc((void*)dst, dstLength);
		if (!uncompressedDst)
		{
			PE_CORE_ERROR("Error uncompressing buffer: realloc() fail");
			free(dst);
			return Buffer();
		}

		dst = nullptr;

		return Buffer(uncompressedDst, dstLength);
	}

	bool BinarySerializer::WriteBuffer(std::ofstream& fout, const Buffer buffer)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(fout.is_open(), "File stream must be open");

		if (!buffer) { return false; }

		fout.write(buffer.As<char>(), buffer.Size());

		return true;
	}

	Buffer BinarySerializer::ReadBuffer(std::ifstream& fin, uint64_t size)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(fin.is_open(), "File stream must be open");

		char* data = new char[size];
		fin.read(data, size);

		return Buffer(data, size);
	}
}