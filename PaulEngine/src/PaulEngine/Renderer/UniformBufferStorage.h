#pragma once
#include "PaulEngine/Core/Core.h"
#include "Buffer.h"

namespace PaulEngine
{
	class UniformBufferStorage
	{
	public:
		struct BufferElement
		{
			BufferElement(std::string name = "null", ShaderDataType type = ShaderDataType::None) : Name(name), Type(type) {
				Size = ShaderDataTypeSize(type);
			}
			std::string Name = "null";
			uint32_t Size = 0;
			uint32_t Offset = 0;
			ShaderDataType Type = ShaderDataType::None;
		};

		virtual ~UniformBufferStorage() {}

		size_t Size() const { return m_Buffer.size(); };

		virtual void UploadStorage() = 0;
		virtual void UploadStorageForced() = 0;
		virtual void Bind(uint32_t binding) = 0;
		virtual void Bind() = 0;

		template <typename T>
		bool SetLocalData(const std::string& name, T data)
		{
			// Find member
			auto it = m_Layout.find(name);
			if (it == m_Layout.end())
			{
				PE_CORE_ERROR("Error writing to member with name '{0}': Member does not exist", name);
				return false;
			}

			const BufferElement& e = m_Layout.at(name);

			// Check for error in member offset and size
			if (e.Offset + e.Size > m_Buffer.size())
			{
				PE_CORE_ERROR("Error writing to member with name '{0}': Buffer overrun", name);
				return false;
			}

			// Validate template type size
			if (sizeof(T) != e.Size)
			{
				PE_CORE_ERROR("Error writing to member with name '{0}': Template type size does not match member size", name);
				return false;
			}

			uint8_t* begin = &m_Buffer[e.Offset];
			memcpy(begin, &data, e.Size);
			m_IsDirty = true;
			return true;
		}

		template <typename T>
		bool ReadLocalDataAs(const std::string& name, T* out_data)
		{
			// Find member
			auto it = m_Layout.find(name);
			if (it == m_Layout.end())
			{
				PE_CORE_ERROR("Error reading member with name '{0}': Member does not exist", name);
				return false;
			}

			const BufferElement& e = m_Layout.at(name);

			// Validate template type size
			if (sizeof(T) != e.Size)
			{
				PE_CORE_ERROR("Error reading member with name '{0}': Template type size does not match member size", name);
				return false;
			}

			uint8_t* begin = &m_Buffer[e.Offset];
			memcpy(out_data, begin, e.Size);
			return true;
		}

		const std::vector<BufferElement>& GetMembers() const { return m_OrderedMembers; }
		virtual uint32_t GetBinding() const = 0;

		static Ref<UniformBufferStorage> Create(std::vector<BufferElement> layout, uint32_t binding);
	
	protected:
		void InitLayout(std::vector<BufferElement>& layout)
		{
			uint32_t offset = 0;
			for (BufferElement& e : layout)
			{
				e.Offset = offset;
				offset += e.Size;
				m_Layout[e.Name] = e;
				m_OrderedMembers.push_back(e);
			}
			m_Buffer = std::vector<uint8_t>(offset);
		}

		std::vector<BufferElement> m_OrderedMembers;
		std::unordered_map<std::string, BufferElement> m_Layout;
		std::vector<uint8_t> m_Buffer;
		bool m_IsDirty;
	};
}