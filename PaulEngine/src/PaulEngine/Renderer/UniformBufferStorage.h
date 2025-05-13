#pragma once
#include "PaulEngine/Core/Core.h"
#include "Buffer.h"

namespace PaulEngine
{
	class ShaderDataTypeStorageBase
	{
	public:
		virtual ~ShaderDataTypeStorageBase() {}
		virtual ShaderDataType GetType() = 0;
		virtual void* GetData() = 0;
		virtual void SetData(void* data) = 0;
		virtual size_t Size() = 0;
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

		virtual ShaderDataType GetType() override { return m_Type; }
		virtual void* GetData() override { return (void*)m_Data; }
		virtual void SetData(void* data) override { 
			if (m_Data) { delete m_Data; }
			m_Data = (T*)data;
		}
		virtual size_t Size() override { return ShaderDataTypeSize(m_Type); }

	private:
		friend class UniformBufferStorage;
		ShaderDataType m_Type;
		T* m_Data;
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
				}
				else
				{
					PE_CORE_ERROR("Error downcasting shader data type storage with name '{0}'", name);
				}
			}
		}

		virtual Ref<ShaderDataTypeStorageBase> GetLocalData(const std::string& name) = 0;
		virtual void SetLocalData(const std::string& name, void* data) = 0;
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
}