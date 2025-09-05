#include "pepch.h"
#include "LocalShaderBuffer.h"

namespace PaulEngine
{
	LocalShaderBuffer::LocalShaderBuffer()
	{
		m_Buffer = std::vector<uint8_t>(0);
		m_OrderedMembers = std::vector<BufferElement>(0);
	}

	LocalShaderBuffer::LocalShaderBuffer(std::vector<BufferElement> layout)
	{
		InitLayout(layout);
	}

	void LocalShaderBuffer::MemCopy(const void* rawData, size_t size, size_t offset)
	{
		size_t end = offset + size;
		PE_CORE_ASSERT(end <= Size(), "Buffer overrun");

		void* begin = &m_Buffer[offset];
		memcpy(begin, rawData, size);
	}

	void LocalShaderBuffer::InitLayout(std::vector<BufferElement>& layout)
	{
		m_OrderedMembers.clear();
		m_MemberMap.clear();

		size_t offset = 0;
		for (BufferElement& e : layout)
		{
			e.Offset = offset;
			offset += e.Size;
			m_MemberMap[e.Name] = m_OrderedMembers.size();
			m_OrderedMembers.push_back(e);
		}
		m_Buffer = std::vector<uint8_t>(offset);
	}
}