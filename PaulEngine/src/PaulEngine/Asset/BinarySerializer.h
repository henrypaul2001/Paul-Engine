#pragma once
#include "PaulEngine/Renderer/Asset/Texture.h"
#include <filesystem>

namespace PaulEngine
{
	class BinarySerializer
	{
	public:
		// Compress using zlib
		static Buffer CompressBuffer(const Buffer buffer);

		// Uncompress using zlib. Must be a buffer previously compressed by zlib
		static Buffer UncompressBuffer(const Buffer buffer, const size_t expectedSize);

		static bool WriteBuffer(std::ofstream& fout, Buffer& buffer);
		static Buffer ReadBuffer(std::ifstream& fin, uint64_t size);
	};
}