#include "pepch.h"
#include "BinarySerializer.h"

namespace PaulEngine
{
	bool BinarySerializer::WriteBuffer(std::ofstream& fout, Buffer& buffer)
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