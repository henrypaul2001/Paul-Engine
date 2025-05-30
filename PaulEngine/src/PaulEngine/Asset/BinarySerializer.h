#pragma once
#include "PaulEngine/Renderer/Asset/Texture.h"
#include <filesystem>

namespace PaulEngine
{
	class BinarySerializer
	{
	public:
		static bool WriteBuffer(std::ofstream& fout, Buffer& buffer);
		static Buffer ReadBuffer(std::ifstream& fin, uint64_t size);
	};
}