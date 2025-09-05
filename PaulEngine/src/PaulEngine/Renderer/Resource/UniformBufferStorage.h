#pragma once
#include "PaulEngine/Renderer/Resource/LocalShaderBuffer.h"

namespace PaulEngine
{
	class UniformBufferStorage
	{
	public:
		UniformBufferStorage(std::vector<BufferElement> layout) : m_Buffer(layout), m_IsDirty(false) {}
		virtual ~UniformBufferStorage() {}

		inline size_t Size() const { return m_Buffer.Size(); };

		virtual void UploadStorage() = 0;
		virtual void UploadStorageForced() = 0;
		virtual void Bind(uint32_t binding) = 0;
		virtual void Bind() = 0;

		template <typename T>
		bool SetLocalData(const std::string& name, T data)
		{
			if (m_Buffer.SetLocalMember(name, data)) { m_IsDirty = true; return true; }
			return false;
		}

		template <typename T>
		inline bool ReadLocalDataAs(const std::string& name, T& out_data) const
		{
			return m_Buffer.ReadLocalMemberAs(name, out_data);
		}

		// Set entire block of data at once.
		// If used incorrectly, may invalidate buffer layout and produce unexpected values when later calling ReadLocalData
		inline void MemCopy(const void* rawData, size_t size, size_t offset = 0)
		{
			m_Buffer.MemCopy(rawData, size, offset);
		}

		const LocalShaderBuffer& GetLocalBuffer() const { return m_Buffer; }
		const std::vector<BufferElement>& GetMembers() const { return m_Buffer.GetMembers(); }
		virtual uint32_t GetBinding() const = 0;

		static Ref<UniformBufferStorage> Create(std::vector<BufferElement> layout, uint32_t binding);
	
	protected:
		LocalShaderBuffer m_Buffer;
		bool m_IsDirty;
	};
}