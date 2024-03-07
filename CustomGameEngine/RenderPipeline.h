#pragma once
#include <vector>
#include "System.h"
namespace Engine {
	enum RENDER_PIPELINE {
		FORWARD_PIPELINE,
		DEFERRED_PIPELINE
	};

	class RenderPipeline
	{
	public:
		RenderPipeline();
		~RenderPipeline();

		RENDER_PIPELINE virtual Name() = 0;
		void virtual Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) = 0;
	};
}