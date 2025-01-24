#pragma once
#include "RenderPipeline.h"
namespace Engine {
	class ForwardPipeline : public RenderPipeline
	{
	public:
		ForwardPipeline() {}
		~ForwardPipeline() {}

		constexpr const char* PipelineName() const override { return "FORWARD_PIPELINE"; }
		void Run(EntityManagerNew* ecs, LightManager* lightManager, CollisionManager* collisionManager) override;
	private:
		void SceneRenderStep();
		void ScreenTextureStep();
	};
}