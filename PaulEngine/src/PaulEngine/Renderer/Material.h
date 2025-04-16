#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "Shader.h"

#include "ShaderParameterType.h"

namespace PaulEngine
{
	/*
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
		Material(AssetHandle shaderHandle);

		void Bind();
		virtual AssetType GetType() const { return AssetType::Material; }

		void AddParameterType(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data);
		void SetParameter(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data);
		Ref<ShaderParamaterTypeStorageBase> GetParameter(const std::string& name);

		void ClearParameters() { m_ShaderParameters.clear(); }

	private:
		friend class EditorLayer;
		friend class MaterialImporter;
		AssetHandle m_ShaderHandle;
		std::unordered_map<std::string, Ref<ShaderParamaterTypeStorageBase>> m_ShaderParameters;
	};
}