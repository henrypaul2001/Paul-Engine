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

		static bool WriteBuffer(std::ofstream& fout, const Buffer buffer);
		static Buffer ReadBuffer(std::ifstream& fin, uint64_t size);

		// Function to serialize a .passet file including the header
		template <typename T>
		static size_t SerializeAssetBinary(T& asset, std::ostream& stream)
		{
			PE_CORE_ERROR("No asset serializer function defined");
			return 0;
		}

		static bool DeserializePAssetHeader(std::istream& stream, size_t& dataSize, AssetType& assetType);

		// Function to deserialize a .passet file, not including the header
		template <typename T>
		static bool DeserializeAssetBinaryData(T& asset, std::istream& stream)
		{
			PE_CORE_ERROR("No asset deserialize function defined");
			return false;
		}

		template <typename T>
		static inline void WriteBinary(T* data, std::ostream& stream) { stream.write((const char*)data, sizeof(T)); }

		template <typename T>
		static inline T ReadBinary(std::istream& stream)
		{
			char data[sizeof(T)]{};
			stream.read(data, sizeof(T));
			return std::bit_cast<T>(data);
		}
	};
}