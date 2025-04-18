#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "Shader.h"

#include "ShaderParameterType.h"

namespace PaulEngine
{
	class Material : public Asset
	{
	public:
		Material();
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
		friend class CreateMaterialWindow;
		AssetHandle m_ShaderHandle;
		std::unordered_map<std::string, Ref<ShaderParamaterTypeStorageBase>> m_ShaderParameters;
	};
}