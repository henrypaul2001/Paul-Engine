#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "Shader.h"

#include "../Resource/ShaderParameterType.h"

#include <unordered_set>

/*
	TODO: Currently the plan is to support both bindless (indirect material buffer entries) and binding materials at the same time in one class
	Once indirect rendering is no longer an experimental feature, consider whether or not materials should be capable of being both 
	bindless and binding, or if materials in Paul Engine should be purely bindless
*/

namespace PaulEngine
{
	class Material : public Asset
	{
	public:
		Material();
		Material(AssetHandle shaderHandle, bool ignoreDefaultShaderOverride = false);

		void Bind();
		virtual AssetType GetType() const { return AssetType::Material; }
		inline const AssetHandle GetShaderHandle() const { return m_ShaderHandle; }
		RenderPipelineContext GetShaderRendererContext() const;

		// Upload indirect material data (SSBO entries) into their respective, larger material buffers at a specified index
		void BindlessUpload(uint32_t materialIndex);

		void AddBindingParameterType(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data);
		void SetParameter(const std::string& name, Ref<ShaderParamaterTypeStorageBase> data);

		Ref<ShaderParamaterTypeStorageBase> GetParameter(const std::string& name);

		template <typename T>
		T* GetParameter(const std::string& name)
		{
			ShaderParamaterTypeStorageBase* baseParam = GetParameter(name).get();
			if (baseParam)
			{
				T* castedParam = dynamic_cast<T*>(baseParam);
				if (!castedParam)
				{
					PE_CORE_ERROR("Error downcasting material parameter with name '{0}'", name);
				}
				return castedParam;
			}
			return nullptr;
		}

		void ClearParameters() { m_BindingParameters.clear(); }

		void AddBindlessTextureHandlesToSet(std::unordered_set<uint64_t>* handleSet);

	private:
		friend class EditorLayer;
		friend class MaterialImporter;
		friend class CreateMaterialWindow;
		friend class Renderer;
		AssetHandle m_ShaderHandle;
		std::unordered_map<std::string, Ref<ShaderParamaterTypeStorageBase>> m_BindingParameters;
		std::vector<Ref<StorageBufferEntryShaderParameterTypeStorage>> m_IndirectParameters;
	};
}