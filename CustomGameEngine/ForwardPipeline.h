#pragma once
#include "RenderPipeline.h"
namespace Engine {
	class ForwardPipeline : public RenderPipeline
	{
	public:
		ForwardPipeline();
		~ForwardPipeline();

		RENDER_PIPELINE Name() override { return FORWARD_PIPELINE; }
		void Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) override;
	private:
		void SceneRenderStep();
		void ScreenTextureStep();
		void BloomBlurStep();
	};
}