#pragma once
#include "RenderPass.h"
namespace PaulEngine
{
	class FrameRenderer
	{
	public:
		FrameRenderer(std::vector<RenderPass> orderedPasses) : m_OrderedRenderPasses(orderedPasses) {}

		void RenderFrame();

	private:
		std::vector<RenderPass> m_OrderedRenderPasses;
	};
}