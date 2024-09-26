#pragma once
#include "RenderPipeline.h"
namespace Engine {
	class ResourceManager;
	class DeferredPipeline : public RenderPipeline
	{
	public:
		DeferredPipeline();
		~DeferredPipeline();

		RENDER_PIPELINE Name() override { return DEFERRED_PIPELINE; }
		void Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) override;

	protected:
		ResourceManager* resources;

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