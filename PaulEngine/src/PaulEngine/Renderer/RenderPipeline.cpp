#include "pepch.h"
#include "Renderer.h"
#include "RenderPipeline.h"

#include "RenderCommand.h"

namespace PaulEngine
{
	Ref<RenderPipeline> RenderPipeline::Create(RenderPipelineSpecification spec)
	{
		return CreateRef<RenderPipeline>(spec.Params.CullState, spec.Params.DepthState, spec.Params.BlendState, spec.ShaderHandle);
	}

	void RenderPipeline::Bind() const
	{
		PE_PROFILE_FUNCTION();
		if (m_DepthState.Test) { RenderCommand::EnableDepth(); }
		else { RenderCommand::DisableDepth(); }
		RenderCommand::DepthMask(m_DepthState.Write);
		RenderCommand::DepthFunc(m_DepthState.Func);

		RenderCommand::SetFaceCulling(m_CullState);

		if (m_BlendState.Enabled) { RenderCommand::EnableBlend(); }
		else { RenderCommand::DisableBlend(); }

		RenderCommand::BlendFunc(m_BlendState.SrcFactor, m_BlendState.DstFactor);
		RenderCommand::BlendColour(m_BlendState.ConstantColour);
		RenderCommand::BlendEquation(m_BlendState.Equation);

		if (AssetManager::IsAssetHandleValid(m_ShaderHandle)) {
			Ref<Shader> shaderAsset = AssetManager::GetAsset<Shader>(m_ShaderHandle);
			if (shaderAsset) { shaderAsset->Bind(); }
		}
	}

	RenderPipelineHash RenderPipeline::Hash() const
	{
		return std::hash<RenderPipeline>{}(*this);
	}
	RenderPipelineHash RenderPipelineSpecification::Hash() const
	{
		return std::hash<RenderPipelineSpecification>{}(*this);
	}
}