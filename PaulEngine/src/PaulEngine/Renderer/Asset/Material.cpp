#include "pepch.h"
#include "Material.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Renderer.h"

namespace PaulEngine
{
	static bool IsDeferredRenderer() { return Project::GetActive()->GetSpecification().RenderContext == RenderPipelineContext::Deferred; }

	Material::Material() : m_ShaderHandle(0), m_DeferredOverrideShader(0) {}
	Material::Material(AssetHandle shaderHandle) : m_ShaderHandle(shaderHandle), m_DeferredOverrideShader(0) {
		PE_PROFILE_FUNCTION();

		// TODO: revisit this, very messy
		if (shaderHandle == Renderer::GetDefaultLitShader())
		{
			Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
			std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

			m_DeferredOverrideShader = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_gBuffer.glsl", true);
		}
		else if (shaderHandle == Renderer::GetDefaultLitPBRShader())
		{
			Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
			std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

			m_DeferredOverrideShader = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_gBufferPBR.glsl", true);
		}

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
					SamplerCubeShaderParameterTypeSpecification* samplerCubeParam = dynamic_cast<SamplerCubeShaderParameterTypeSpecification*>(parameter.get());
					Ref<SamplerCubeShaderParameterTypeStorage> samplerCubeStorage = CreateRef<SamplerCubeShaderParameterTypeStorage>(0, samplerCubeParam->Binding);
					AddParameterType(strippedName, samplerCubeStorage);
					break;
				}
			}
		}
	}

	void Material::Bind()
	{
		PE_PROFILE_FUNCTION();
		// TODO: revisit this, very messy
		Ref<Shader> shaderAsset = nullptr;
		if (IsDeferredRenderer() && m_DeferredOverrideShader != 0) { shaderAsset = AssetManager::GetAsset<Shader>(m_DeferredOverrideShader); }
		else { shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle); }

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