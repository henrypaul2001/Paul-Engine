#include "pepch.h"
#include "Material.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Renderer.h"

namespace PaulEngine
{
	Material::Material() : m_ShaderHandle(0) {}
	Material::Material(AssetHandle shaderHandle, bool ignoreDefaultShaderOverride) : m_ShaderHandle(shaderHandle) {
		PE_PROFILE_FUNCTION();

		if (!ignoreDefaultShaderOverride)
		{
			Renderer::ValidateDefaultShader(m_ShaderHandle);
		}

		// Generate binding parameters from shader reflection data
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
					AddBindingParameterType(strippedName, uboStorage);
					break;
				}
				case ShaderParameterType::Sampler2D:
				{
					Sampler2DShaderParameterTypeSpecification* samplerParam = dynamic_cast<Sampler2DShaderParameterTypeSpecification*>(parameter.get());
					Ref<Sampler2DShaderParameterTypeStorage> samplerStorage = CreateRef<Sampler2DShaderParameterTypeStorage>(0, samplerParam->Binding);
					AddBindingParameterType(strippedName, samplerStorage);
					break;
				}
				case ShaderParameterType::Sampler2DArray:
				{
					Sampler2DArrayShaderParameterTypeSpecification* samplerArrayParam = dynamic_cast<Sampler2DArrayShaderParameterTypeSpecification*>(parameter.get());
					Ref<Sampler2DArrayShaderParameterTypeStorage> samplerArrayStorage = CreateRef<Sampler2DArrayShaderParameterTypeStorage>(0, samplerArrayParam->Binding);
					AddBindingParameterType(strippedName, samplerArrayStorage);
					break;
				}
				case ShaderParameterType::SamplerCube:
				{
					SamplerCubeShaderParameterTypeSpecification* samplerCubeParam = dynamic_cast<SamplerCubeShaderParameterTypeSpecification*>(parameter.get());
					Ref<SamplerCubeShaderParameterTypeStorage> samplerCubeStorage = CreateRef<SamplerCubeShaderParameterTypeStorage>(0, samplerCubeParam->Binding);
					AddBindingParameterType(strippedName, samplerCubeStorage);
					break;
				}
				case ShaderParameterType::SamplerCubeArray:
				{
					SamplerCubeArrayShaderParameterTypeSpecification* samplerCubeArrayParam = dynamic_cast<SamplerCubeArrayShaderParameterTypeSpecification*>(parameter.get());
					Ref<SamplerCubeArrayShaderParameterTypeStorage> samplerCubeArrayStorage = CreateRef<SamplerCubeArrayShaderParameterTypeStorage>(0, samplerCubeArrayParam->Binding);
					AddBindingParameterType(strippedName, samplerCubeArrayStorage);
					break;
				}
			}
		}
		
		// Generate local bindless material buffer entries
		const std::vector<Ref<StorageBufferShaderParameterTypeSpecification>>& bindlessParams = shaderAsset->GetMaterialBufferSpecs();
		const std::vector<Ref<ShaderStorageBuffer>>& storageBuffers = shaderAsset->GetMaterialBuffers();
		for (uint32_t i = 0; i < bindlessParams.size(); i++)
		{
			const Ref<StorageBufferShaderParameterTypeSpecification>& ssboParam = bindlessParams[i];
			std::string strippedName = ssboParam->Name.substr(5, ssboParam->Name.size() - 5);

			int32_t capacity = (ssboParam->DynamicArrayStart > -1) ? -1 : ssboParam->Size;

			Ref<StorageBufferEntryShaderParameterTypeStorage> ssboEntry = CreateRef<StorageBufferEntryShaderParameterTypeStorage>(ssboParam->BufferLayout, storageBuffers[i], capacity);
			AddBindingParameterType(strippedName, ssboEntry);
			m_IndirectParameters.push_back(ssboEntry);
		}
	}

	void Material::Bind()
	{
		PE_PROFILE_FUNCTION();
		Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
		if (shaderAsset) {
			shaderAsset->Bind();

			for (auto& it : m_BindingParameters) {
				it.second->Bind();
			}

			return;
		}
		PE_CORE_ERROR("Error retrieving material shader with handle '{0}'", (uint64_t)m_ShaderHandle);
		return;
	}

	void Material::BindlessUpload(uint32_t materialIndex)
	{
		for (auto indirectEntry : m_IndirectParameters)
		{
			indirectEntry->BindlessUpload(materialIndex);
		}
	}

	RenderPipelineContext Material::GetShaderRendererContext() const
	{
		PE_PROFILE_FUNCTION();
		Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
		if (shaderAsset)
		{
			return shaderAsset->GetRenderPipelineContext();
		}
		return RenderPipelineContext::Undefined;
	}

	void Material::AddBindingParameterType(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data)
	{
		PE_PROFILE_FUNCTION();
		auto it = m_BindingParameters.find(name);
		if (it == m_BindingParameters.end()) {
			m_BindingParameters[name] = data;
			return;
		}
		PE_CORE_ERROR("Name '{0}' already exists in shader parameters map", name);
	}

	void Material::SetParameter(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data)
	{
		PE_PROFILE_FUNCTION();
		auto it = m_BindingParameters.find(name);
		if (it != m_BindingParameters.end()) {
			it->second = data;
		}
		else {
			PE_CORE_ERROR("Name '{0}' not found in shader parameters", name);
		}
	}

	Ref<ShaderParamaterTypeStorageBase> Material::GetParameter(const std::string& name)
	{
		PE_PROFILE_FUNCTION();
		auto it = m_BindingParameters.find(name);
		if (it != m_BindingParameters.end()) {
			return it->second;
		}
		else {
			PE_CORE_ERROR("Name '{0}' not found in shader parameters", name);
			return nullptr;
		}
	}
}