#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "Shader.h"

namespace PaulEngine
{
	/*
		Material:
			std::string m_Name;
			AssetHandle m_Shader;
			std::unordered_map<std::string, Ref<ShaderParamaterStorageBase>> m_ParamaterMap;

			template <typename T>
			void AddParamater(std::string name, ShaderParamaterType type, T* data)
			{
				m_ParamaterMap[name] = CreateRef<ShaderParmaterStorage<T>>(type, data);
			}

		ShaderParamaterStorageBase
		{
			virtual void* GetData() = 0;
			virtual size_t Size() = 0;
			virtual ShaderParamaterType GetShaderParamaterType() = 0;
		}

		template <typename T>
		ShaderParamaterStorage : public ShaderParamaterStorageBase
		{
			virtual void* GetData() override { return (void*)m_Data; }
			virtual size_t Size() override { return sizeof(m_Data); }
			virtual ShaderParamaterType GetShaderParamaterType() override { return m_Type; }

			ShaderParamaterType m_Type;
			T* m_Data;

			~ShaderParamaterType()
			{
				delete m_Data;
			}
		}

		// Material file
		name: "TestMaterial"
		shader: (AssetHandle)
		shaderParams
		[
			name: "u_MaterialValues"
			type: "UBO"
			value:
				[
					binding: 0,
					layout
					[
						["vec4", "colour", [1.0, 0.0, 0.0, 1.0]],
						["float", "roughness", 1.0],
						["float", "metalness", 0.0]
					]
				]
			,
			name: "materialMaps"
			type: "sampler2DArray"
			value:
				location: 0
			,
			name: "displacementMap"
			type: "sampler2D"
			value:
				location: 1
		]

		enum class ShaderDataType
		{
			None = 0,
			Float, Float2, Float3, Float4,
			Mat3, Mat4,
			Int, Int2, Int3, Int4,
			Bool
		};

		class ShaderDataTypeStorageBase
		{
			virtual ShaderDataType GetType() = 0;
			virtual void* GetData() = 0;
			virtual size_t Size() = 0;
		}

		template <typename T>
		class ShaderDataTypeStorage
		{
			virtual GetType() override { return m_Type; }
			virtual GetData() override { return (void*)m_Data; }
			virtual Size() override { return sizeof(m_Data); }

			ShaderDataType m_Type;
			T* m_Data;
		}

		enum class ShaderParamaterType
		{
			None = 0,
			UBO, (T = UniformBufferStorage)
			Sampler2D, (T = uint32_t)
			Sampler2DArray (T = uint32_t)
		};

		ShaderParamaterType StringToType(const std::string& input)... return type;

		class UniformBufferStorage
		{
			uint32_t m_Binding;
			std::unordered_map<std::string, Ref<ShaderDataTypeStorageBase>> m_LayoutStorage;
		
			template <typename T>
			void AddDataType(std::string name, ShaderDataType type, T* data)
			{
				m_LayoutStorage[name] = CreateRef<ShaderDataTypeStorage<T>>(type, data);
			}

			void UploadStorage()
			{
				for (ShaderDataTypeStorage : m_LayoutStorage) {
					glNamedBufferSubData(m_RendererID, offset, size, data);
				}
			}
		}

		// How do we use it?
		
		// Create material
		Material(AssetHandle shader);

		UniformBufferStorage* bufferStorage = new UniformBufferStorage();
		glm::vec4* data = new glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		bufferStorage->AddDataType<glm::vec4>("colour", ShaderDataType::float4, data);
		float* data = new float(1.0f);
		bufferStorage->AddDataType<float>("roughness", ShaderDataType::float, data);
		float* data = new float(1.0f);
		bufferStorage->AddDataType<float>("metalness", ShaderDataType::float, data);

		material.AddParamater("u_MaterialValues", ShaderParamaterType::UBO, bufferStorage);
		
		uint32_t* data = new uint32_t(0);
		material.AddParamater("materialMaps", ShaderParamaterType::Sampler2DArray, data);
		
		uint32_t* data = new uint32_t(1);
		material.AddParamater("displacementMap", ShaderParamaterType::Sampler2D, data);
	*/

	class Material : public Asset
	{
	public:
		Material();

		void Bind();

		virtual AssetType GetType() const { return AssetType::Material; }

	private:
		Ref<Shader> m_Shader;
		//Ref<ShaderParameters> m_Paramaters;
	};
}