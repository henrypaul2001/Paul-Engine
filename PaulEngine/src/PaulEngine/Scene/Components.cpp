#include "pepch.h"
#include "Components.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Asset/Material.h"

namespace PaulEngine
{
	void ApplyForwardCompatibleTag(Entity targetEntity)
	{
		if (targetEntity.HasComponent<DeferredCompatibleMaterialTag>())
		{
			targetEntity.RemoveComponent<DeferredCompatibleMaterialTag>();
		}
		if (!targetEntity.HasComponent<ForwardCompatibleMaterialTag>())
		{
			targetEntity.AddComponent<ForwardCompatibleMaterialTag>();
		}
	}
	void ApplyDeferredCompatibleTag(Entity targetEntity)
	{
		if (targetEntity.HasComponent<ForwardCompatibleMaterialTag>())
		{
			targetEntity.RemoveComponent<ForwardCompatibleMaterialTag>();
		}
		if (!targetEntity.HasComponent<DeferredCompatibleMaterialTag>())
		{
			targetEntity.AddComponent<DeferredCompatibleMaterialTag>();
		}
	}

	void ComponentMeshRenderer::SetMaterial(Entity targetEntity, AssetHandle materialHandle)
	{
		PE_CORE_ASSERT(targetEntity.HasComponent<ComponentMeshRenderer>(), "Target entity does not have a mesh component to modify");
		ComponentMeshRenderer& meshComponent = targetEntity.GetComponent<ComponentMeshRenderer>();
		meshComponent.m_MaterialHandle = materialHandle;

		Ref<Material> materialAsset = AssetManager::GetAsset<Material>(materialHandle);
		RenderPipelineContext shaderContext = RenderPipelineContext::Undefined;
		if (materialAsset)
		{
			shaderContext = materialAsset->GetShaderRendererContext();
		}

		switch (shaderContext)
		{
		case RenderPipelineContext::Undefined:
			PE_CORE_ERROR("Undefined shader renderer context");
			meshComponent.m_MaterialHandle = 0;
			ApplyForwardCompatibleTag(targetEntity);
			break;
		case RenderPipelineContext::Forward:
			ApplyForwardCompatibleTag(targetEntity);
			break;
		case RenderPipelineContext::Deferred:
			ApplyDeferredCompatibleTag(targetEntity);
			break;
		}
	}
}