#pragma once
#include "PaulEngine/Renderer/RenderPipeline.h"

namespace PaulEngine
{
	class OpenGLRenderPipeline : public RenderPipeline
	{
	public:
		OpenGLRenderPipeline(FaceCulling cullState, DepthState depthState, BlendState blendState, AssetHandle material) : RenderPipeline(cullState, depthState, blendState, material) {}

		virtual void Bind() const override;

	private:
		void Blend(BlendState blendState) const;
	};
}