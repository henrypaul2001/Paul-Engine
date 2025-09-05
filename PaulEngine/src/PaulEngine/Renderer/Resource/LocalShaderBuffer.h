#pragma once
#include "PaulEngine/Renderer/Resource/Buffer.h"

namespace PaulEngine
{
	class LocalShaderBuffer
	{
	public:
		LocalShaderBuffer();
		LocalShaderBuffer(std::vector<BufferElement> layout);

		inline size_t Size() const { return m_Buffer.size(); }
		inline const uint8_t* Data() const { return m_Buffer.data(); }
		inline const std::vector<BufferElement>& GetMembers() const { return m_OrderedMembers; }

		template<typename T>
		bool SetLocalMember(const std::string& memberName, T& data)
		{
			// Find member
			auto it = m_MemberMap.find(memberName);
			if (it == m_MemberMap.end())
			{
				PE_CORE_ERROR("Error writing to member with name '{0}': Member does not exist", memberName.c_str());
				return false;
			}

			const BufferElement& e = m_OrderedMembers[it->second];

			// TODO: potentially unnecessary validation. buffer layout is immutable and the buffer size is determined by that layout so I'm pretty sure it's not possible to trigger this error
			// Check for error in member offset and size
			if (e.Offset + e.Size > Size())
			{
				PE_CORE_ERROR("Error writing to member with name '{0}': Buffer overrun", memberName.c_str());
				return false;
			}

			// Validate template type size
			if (sizeof(T) != e.Size)
			{
				PE_CORE_ERROR("Error writing to member with name '{0}': Template type size does not match member size", memberName.c_str());
				return false;
			}

			void* begin = &m_Buffer[e.Offset];
			memcpy(begin, (const void*)&data, e.Size);

			return true;
		}

		template<typename T>
		bool ReadLocalMemberAs(const std::string& memberName, T& out_data) const
		{
			// Find member
			auto it = m_MemberMap.find(memberName);
			if (it == m_MemberMap.end())
			{
				PE_CORE_ERROR("Error reading member with name '{0}': Member does not exist", memberName.c_str());
				return false;
			}

			const BufferElement& e = m_OrderedMembers[it->second];

			// Validate template type size
			if (sizeof(T) != e.Size)
			{
				PE_CORE_ERROR("Error reading member with name '{0}': Template type size does not match member size", memberName.c_str());
				return false;
			}

			const void* begin = &m_Buffer[e.Offset];
			memcpy((void*)&out_data, begin, e.Size);
			return true;
		}

		// Set entire block of data at once.
		// If used incorrectly, may invalidate buffer layout and produce unexpected values when later calling ReadLocalMember
		void MemCopy(const void* rawData, size_t size, size_t offset = 0);

	private:
		std::vector<BufferElement> m_OrderedMembers;
		std::unordered_map<std::string, uint32_t> m_MemberMap; // <name, index>
		std::vector<uint8_t> m_Buffer;

		void InitLayout(std::vector<BufferElement>& layout);
	};
}