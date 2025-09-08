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

	std::string GetCommonPrefix(std::vector<BufferElement>& layout)
	{
		if (layout.size() == 0)
		{
			return "";
		}

		std::string longestString = layout[0].Name;
		for (size_t i = 1; i < layout.size(); i++)
		{
			if (layout[i].Name.size() > longestString.size()) { longestString = layout[i].Name; }
		}

		size_t firstDot = longestString.find_first_of('.');
		if (firstDot == std::string::npos) { return ""; }

		std::string commonPrefix = longestString.substr(0, firstDot + 1);
		for (size_t i = 1; i < layout.size(); i++)
		{
			size_t newFirstDot = layout[i].Name.find_first_of('.');
			if (newFirstDot == std::string::npos) { return ""; }

			if (layout[i].Name.substr(0, newFirstDot + 1) != commonPrefix) { return ""; }
		}

		return commonPrefix;
	}

	void LocalShaderBuffer::InitLayout(std::vector<BufferElement>& layout)
	{
		m_OrderedMembers.clear();
		m_MemberMap.clear();

		size_t offset = 0;
		for (BufferElement& e : layout)
		{
			if (m_MemberMap.find(e.Name) == m_MemberMap.end())
			{
				e.Offset = offset;
				offset += e.Size;
				size_t memberIndex = m_OrderedMembers.size();
				m_MemberMap[e.Name] = memberIndex;
				if (IsTextureShaderDataType(e.Type))
				{
					m_TextureMemberNames.push_back(e.Name);
				}
				m_OrderedMembers.push_back(e);
			}
		}
		m_Buffer = std::vector<uint8_t>(offset);

		m_CommonPrefix = GetCommonPrefix(m_OrderedMembers);
	}
}