#pragma once
#include "PaulEngine/Renderer/RenderPipeline.h"

namespace PaulEngine
{
	class OpenGLRenderPipeline : public RenderPipeline
	{
	public:
		OpenGLRenderPipeline(FaceCulling cullState, DepthState depthState, bool blend, AssetHandle material) : RenderPipeline(cullState, depthState, blend, material) {}

		virtual void Bind() const override;
	};
}