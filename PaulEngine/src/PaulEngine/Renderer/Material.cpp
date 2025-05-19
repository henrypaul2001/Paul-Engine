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
			if (parameter->Name.substr(0, 4) != "Mat_") { continue; }

			std::string strippedName = parameter->Name.substr(4, parameter->Name.size() - 4);
			switch (parameter->Type())
			{
				case ShaderParameterType::None:
				{
					break;
				}
				case ShaderParameterType::UBO:
				{
					UBOShaderParameterTypeSpecification* uboParam = dynamic_cast<UBOShaderParameterTypeSpecification*>(parameter.get());
					Ref<UBOShaderParameterTypeStorage> uboStorage = CreateRef<UBOShaderParameterTypeStorage>(uboParam->BufferLayout, uboParam->Binding);
					AddParameterType(strippedName, uboStorage);
					break;
				}
				case ShaderParameterType::Sampler2D:
				{
					Sampler2DShaderParameterTypeSpecification* samplerParam = dynamic_cast<Sampler2DShaderParameterTypeSpecification*>(parameter.get());
					Ref<Sampler2DShaderParameterTypeStorage> samplerStorage = CreateRef<Sampler2DShaderParameterTypeStorage>(0, samplerParam->Binding);
					AddParameterType(strippedName, samplerStorage);
					break;
				}
				case ShaderParameterType::Sampler2DArray:
				{
					Sampler2DArrayShaderParameterTypeSpecification* samplerArrayParam = dynamic_cast<Sampler2DArrayShaderParameterTypeSpecification*>(parameter.get());
					Ref<Sampler2DArrayShaderParameterTypeStorage> samplerArrayStorage = CreateRef<Sampler2DArrayShaderParameterTypeStorage>(0, samplerArrayParam->Binding);
					AddParameterType(strippedName, samplerArrayStorage);
					break;
				}
				case ShaderParameterType::SamplerCube:
				{
					PE_CORE_WARN("SamplerCube material parameter not yet supported");
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