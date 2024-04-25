#pragma once
#include "RenderPipeline.h"
namespace Engine {
	class DeferredPipeline : public RenderPipeline
	{
	public:
		DeferredPipeline(bool pbr = false);
		~DeferredPipeline();

		bool PBR() { return pbr; }
		bool PBR(bool PBR) { pbr = pbr; }

		RENDER_PIPELINE Name() override { return DEFERRED_PIPELINE; }
		void Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) override;
	private:
		bool pbr;
	};
}