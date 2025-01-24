#pragma once
#include "RenderPipeline.h"
namespace Engine {
	class DeferredPipeline : public RenderPipeline
	{
	public:
		DeferredPipeline() {}
		~DeferredPipeline() {}

		constexpr const char* PipelineName() const override { return "DEFERRED_PIPELINE"; }
		void Run(EntityManagerNew* ecs, LightManager* lightManager, CollisionManager* collisionManager) override;

	protected:
		void SwapScreenTextures() {
			unsigned int temp = activeScreenTexture;
			activeScreenTexture = alternateScreenTexture;
			alternateScreenTexture = temp;

			temp = activeBloomTexture;
			activeBloomTexture = alternateBloomTexture;
			alternateBloomTexture = temp;
		}

		unsigned int activeScreenTexture;
		unsigned int alternateScreenTexture;

		unsigned int activeBloomTexture;
		unsigned int alternateBloomTexture;
	};
}