#pragma once
#include <vector>
#include "System.h"
namespace Engine {
	class RenderPipeline
	{
	public:
		RenderPipeline();
		~RenderPipeline();

		void virtual Run(std::vector<System*> renderSystems, std::vector<Entity*> entities) = 0;
	};
}