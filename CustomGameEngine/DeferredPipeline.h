#pragma once
#include "RenderPipeline.h"
namespace Engine {
	class DeferredPipeline : public RenderPipeline
	{
	public:
		DeferredPipeline();
		~DeferredPipeline();

		RENDER_PIPELINE Name() override { return DEFERRED_PIPELINE; }
		void Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) override;
	};
}