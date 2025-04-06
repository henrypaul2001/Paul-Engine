#include "pepch.h"
#include "Material.h"

#include "PaulEngine/Asset/AssetManager.h"

namespace PaulEngine
{
	Material::Material(AssetHandle shaderHandle) : m_ShaderHandle(shaderHandle) {}

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