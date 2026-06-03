#include "pepch.h"
#include "Material.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Renderer.h"
#include "PaulEngine/Renderer/Renderer2D.h"

#include "Texture.h"

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
			AddIndirectParameterType(strippedName, ssboEntry);
		}
	}

	void Material::Bind()
	{
		PE_PROFILE_FUNCTION();
		Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
		if (shaderAsset) {
			shaderAsset->Bind();

			BindingUpload();

			return;
		}
		PE_CORE_ERROR("Error retrieving material shader with handle '{0}'", (uint64_t)m_ShaderHandle);
		return;
	}

	void Material::BindlessUpload(uint32_t materialIndex)
	{
		for (auto& indirectEntry : m_IndirectParameters)
		{
			indirectEntry->BindlessUpload(materialIndex);
		}
	}

	void Material::BindingUpload(bool includeBindless)
	{
		for (auto& it : m_BindingParameters) {
			it->Bind();
		}

		if (includeBindless)
		{
			for (auto& indirectEntry : m_IndirectParameters)
			{
				indirectEntry->Bind();
			}
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
		auto it = m_ParameterMap.find(name);
		if (it == m_ParameterMap.end()) {
			m_ParameterMap[name] = { true, m_BindingParameters.size() };
			m_BindingParameters.push_back(data);
			return;
		}
		PE_CORE_ERROR("Name '{0}' already exists in shader parameters map", name);
	}

	void Material::AddIndirectParameterType(const std::string& name, Ref<StorageBufferEntryShaderParameterTypeStorage> data)
	{
		PE_PROFILE_FUNCTION();
		auto it = m_ParameterMap.find(name);
		if (it == m_ParameterMap.end()) {
			m_ParameterMap[name] = { false, m_IndirectParameters.size() };
			m_IndirectParameters.push_back(data);
			return;
		}
		PE_CORE_ERROR("Name '{0}' already exists in shader parameters map", name);
	}

	void Material::SetParameter(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data)
	{
		PE_PROFILE_FUNCTION();
		auto it = m_ParameterMap.find(name);
		if (it != m_ParameterMap.end()) {
			if (it->second.IsBinding)
			{
				m_BindingParameters[it->second.Index] = data;
			}
			else
			{
				m_IndirectParameters[it->second.Index] = std::static_pointer_cast<StorageBufferEntryShaderParameterTypeStorage>(data);
			}
		}
		else {
			PE_CORE_ERROR("Name '{0}' not found in shader parameters", name);
		}
	}

	void Material::AddBindlessTextureHandlesToSet(std::unordered_set<uint64_t>* handleSet)
	{
		for (Ref<StorageBufferEntryShaderParameterTypeStorage> param : m_IndirectParameters)
		{
			const LocalShaderBuffer& localBuffer = param->GetLocalBuffer();
			std::vector<AssetHandle> textureAssetHandles = localBuffer.GetTextureMemberHandles();

			for (const AssetHandle& handle : textureAssetHandles)
			{
				uint64_t deviceHandle = DeviceHandleTracker::AssetHandleToDeviceHandle(handle);
				if (deviceHandle != 0)
				{ 
					handleSet->insert(deviceHandle);
				}
				else
				{
					deviceHandle = DeviceHandleTracker::AssetHandleToDeviceHandle(Renderer2D::GetWhiteTexture()->Handle);
					handleSet->insert(deviceHandle);
				}
			}
		}
	}

	void Material::LoadBindlessTextures()
	{
		for (Ref<StorageBufferEntryShaderParameterTypeStorage> param : m_IndirectParameters)
		{
			const LocalShaderBuffer& localBuffer = param->GetLocalBuffer();
			std::vector<AssetHandle> textureAssetHandles = localBuffer.GetTextureMemberHandles();

			for (const AssetHandle& handle : textureAssetHandles)
			{
				AssetManager::GetAsset<Texture>(handle);
			}
		}
	}

	Ref<ShaderParamaterTypeStorageBase> Material::GetParameter(const std::string& name)
	{
		PE_PROFILE_FUNCTION();
		auto it = m_ParameterMap.find(name);
		if (it != m_ParameterMap.end()) {
			if (it->second.IsBinding)
			{
				return m_BindingParameters[it->second.Index];
			}
			else
			{
				return m_IndirectParameters[it->second.Index];
			}
		}
		else {
			PE_CORE_ERROR("Name '{0}' not found in shader parameters", name);
			return nullptr;
		}
	}
}