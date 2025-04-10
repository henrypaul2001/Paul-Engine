#pragma once
#include "PaulEngine/Renderer/RenderPipeline.h"

namespace PaulEngine
{
	class OpenGLRenderPipeline : public RenderPipeline
	{
	public:
		OpenGLRenderPipeline(FaceCulling cullState, DepthState depthState, AssetHandle material) : RenderPipeline(cullState, depthState, material) {}

		virtual void Bind() const override;
	};
}