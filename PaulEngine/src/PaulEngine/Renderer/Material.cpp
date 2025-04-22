#include "pepch.h"
#include "Material.h"

#include "PaulEngine/Asset/AssetManager.h"

namespace PaulEngine
{
	Material::Material() : m_ShaderHandle(0) {}
	Material::Material(AssetHandle shaderHandle) : m_ShaderHandle(shaderHandle) {
		PE_PROFILE_FUNCTION();
		// Generate parameters from shader reflection data
		Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
		PE_CORE_ASSERT(shaderAsset, "Invalid shader asset");

		for (auto& parameter : shaderAsset->GetReflectionData())
		{
			// Temporary way of determining which parameters are material asset parameters
			if (parameter->Name == "Camera") { continue; }
			if (parameter->Name == "u_Textures") { continue; }
			if (parameter->Name == "MeshSubmission") { continue; }
			if (parameter->Name == "SceneData") { continue; }

			switch (parameter->Type())
			{
				case ShaderParameterType::None:
				{
					break;
				}
				case ShaderParameterType::UBO:
				{
					UBOShaderParameterTypeSpecification* uboParam = dynamic_cast<UBOShaderParameterTypeSpecification*>(parameter.get());
					Ref<UBOShaderParameterTypeStorage> uboStorage = CreateRef<UBOShaderParameterTypeStorage>(uboParam->Size, uboParam->Binding);
					for (auto& dataType : uboParam->BufferLayout) {
						const std::string& name = dataType.Name;
						const ShaderDataType type = dataType.Type;

						switch (dataType.Type)
						{
							case ShaderDataType::None:
								continue;
								break;
							case ShaderDataType::Float:
							{
								float* data = new float();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<float>>(type, data));
								break;
							}
							case ShaderDataType::Float2:
							{
								glm::vec2* data = new glm::vec2();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::vec2>>(type, data));
								break;
							}
							case ShaderDataType::Float3:
							{
								glm::vec3* data = new glm::vec3();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::vec3>>(type, data));
								break;
							}
							case ShaderDataType::Float4:
							{
								glm::vec4* data = new glm::vec4();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::vec4>>(type, data));
								break;
							}
							case ShaderDataType::Mat3:
							{
								glm::mat3* data = new glm::mat3();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::mat3>>(type, data));
								break;
							}
							case ShaderDataType::Mat4:
							{
								glm::mat4* data = new glm::mat4();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::mat4>>(type, data));
								break;
							}
							case ShaderDataType::Int:
							{
								int* data = new int();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<int>>(type, data));
								break;
							}
							case ShaderDataType::Int2:
							{
								glm::ivec2* data = new glm::ivec2();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::ivec2>>(type, data));
								break;
							}
							case ShaderDataType::Int3:
							{
								glm::ivec3* data = new glm::ivec3();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::ivec3>>(type, data));
								break;
							}
							case ShaderDataType::Int4:
							{
								glm::ivec4* data = new glm::ivec4();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<glm::ivec4>>(type, data));
								break;
							}
							case ShaderDataType::Bool:
							{
								bool* data = new bool();
								uboStorage->UBO()->AddDataType(name, CreateRef<ShaderDataTypeStorage<bool>>(type, data));
								break;
							}
						}
					}
					AddParameterType(parameter->Name, uboStorage);
					break;
				}
				case ShaderParameterType::Sampler2D:
				{
					Sampler2DShaderParameterTypeSpecification* samplerParam = dynamic_cast<Sampler2DShaderParameterTypeSpecification*>(parameter.get());
					Ref<Sampler2DShaderParameterTypeStorage> samplerStorage = CreateRef<Sampler2DShaderParameterTypeStorage>(0, samplerParam->Binding);
					AddParameterType(parameter->Name, samplerStorage);
					break;
				}
				case ShaderParameterType::Sampler2DArray:
				{
					Sampler2DArrayShaderParameterTypeSpecification* samplerArrayParam = dynamic_cast<Sampler2DArrayShaderParameterTypeSpecification*>(parameter.get());
					Ref<Sampler2DArrayShaderParameterTypeStorage> samplerArrayStorage = CreateRef<Sampler2DArrayShaderParameterTypeStorage>(0, samplerArrayParam->Binding);
					AddParameterType(parameter->Name, samplerArrayStorage);
					break;
				}
			}
		}
	}

	void Material::Bind()
	{
		PE_PROFILE_FUNCTION();
		Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
		if (shaderAsset) {
			shaderAsset->Bind();

			for (auto& it : m_ShaderParameters) {
				it.second->Bind();
			}

			return;
		}
		PE_CORE_ERROR("Error retrieving material shader with handle '{0}'", (uint64_t)m_ShaderHandle);
		return;
	}

	void Material::AddParameterType(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data)
	{
		PE_PROFILE_FUNCTION();
		if (m_ShaderParameters.find(name) == m_ShaderParameters.end()) {
			m_ShaderParameters[name] = data;
			return;
		}
		PE_CORE_ERROR("Name '{0}' already exists in shader parameters map", name);
	}

	void Material::SetParameter(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data)
	{
		PE_PROFILE_FUNCTION();
		if (m_ShaderParameters.find(name) != m_ShaderParameters.end()) {
			m_ShaderParameters[name] = data;
		}
		else {
			PE_CORE_ERROR("Name '{0}' not found in shader parameters", name);
		}
	}

	Ref<ShaderParamaterTypeStorageBase> Material::GetParameter(const std::string& name)
	{
		PE_PROFILE_FUNCTION();
		
		if (m_ShaderParameters.find(name) != m_ShaderParameters.end()) {
			return m_ShaderParameters.at(name);
		}
		else {
			PE_CORE_ERROR("Name '{0}' not found in shader parameters", name);
			return nullptr;
		}
	}
}