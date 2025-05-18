#pragma once
#include "PaulEngine/Core/Core.h"
#include "Buffer.h"

namespace PaulEngine
{
	class ShaderDataTypeStorageBase
	{
	public:
		virtual ~ShaderDataTypeStorageBase() {}
		virtual ShaderDataType GetType() const = 0;
		virtual const void* GetData() const = 0;
		virtual size_t Size() = 0;
		virtual bool IsDirty() const = 0;
		virtual void SetDirtyFlag(bool dirty) = 0;

		template <typename T>
		const T* GetData()
		{
			return static_cast<const T*>(GetData());
		}
	};

	template <typename T>
	class ShaderDataTypeStorage : public ShaderDataTypeStorageBase
	{
	public:
		ShaderDataTypeStorage(ShaderDataType type, T* data) : m_Type(type), m_Data(data) {}
		~ShaderDataTypeStorage() {
			if (m_Data) {
				delete m_Data;
			}
		}

		virtual ShaderDataType GetType() const override { return m_Type; }
		virtual const void* GetData() const override { return (void*)m_Data; }
		virtual size_t Size() override { return ShaderDataTypeSize(m_Type); }
		virtual bool IsDirty() const override { return m_Dirty; }
		virtual void SetDirtyFlag(bool dirty) override { m_Dirty = dirty; }

	private:
		friend class UniformBufferStorage;
		ShaderDataType m_Type;
		T* m_Data;
		bool m_Dirty;
	};

	class UniformBufferStorage
	{
	public:
		virtual ~UniformBufferStorage() {}

		virtual uint32_t GetBinding() const = 0;

		template <typename T>
		void SetLocalData(const std::string& name, T data)
		{
			ShaderDataTypeStorageBase* baseData = GetLocalData(name).get();
			if (baseData)
			{
				ShaderDataTypeStorage<T>* casted = dynamic_cast<ShaderDataTypeStorage<T>*>(baseData);
				if (casted)
				{
					*casted->m_Data = data;
					casted->SetDirtyFlag(true);
				}
				else
				{
					PE_CORE_ERROR("Error downcasting shader data type storage with name '{0}'", name);
				}
			}
		}

		template <typename T>
		ShaderDataTypeStorage<T>* GetLocalData(const std::string& name)
		{
			ShaderDataTypeStorageBase* baseData = GetLocalData(name).get();
			if (baseData)
			{
				ShaderDataTypeStorage<T>* casted = dynamic_cast<ShaderDataTypeStorage<T>*>(baseData);
				if (!casted)
				{
					PE_CORE_ERROR("Error downcasting shader data type storage with name '{0}'", name);
				}
				return casted;
			}
		}

		virtual Ref<ShaderDataTypeStorageBase> GetLocalData(const std::string& name) = 0;
		virtual void AddDataType(const std::string& name, Ref<ShaderDataTypeStorageBase> data) = 0;
		virtual void UploadStorage() = 0;
		virtual void Bind(uint32_t binding) = 0;
		virtual void Bind() = 0;

		struct LayoutElement
		{
			std::string Name;
			Ref<ShaderDataTypeStorageBase> Data;
		};

		virtual std::vector<LayoutElement>& GetLayoutStorage() = 0;
		virtual const std::vector<LayoutElement>& GetLayoutStorage() const = 0;

		static Ref<UniformBufferStorage> Create(size_t size, uint32_t binding);
	};


	class UniformBufferStorageNew
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

		UniformBufferStorageNew(std::vector<BufferElement> layout)
		{
			uint32_t offset = 0;
			for (BufferElement& e : layout)
			{
				e.Offset = offset;
				offset += e.Size;
				m_Layout[e.Name] = e;
			}
		}
		virtual ~UniformBufferStorageNew();

		size_t Size() const { return m_Size; }

	private:
		std::unordered_map<std::string, BufferElement> m_Layout;
		std::vector<uint8_t> m_Buffer;
		size_t m_Size;
	};
}