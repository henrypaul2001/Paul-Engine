#pragma once
#include "RenderPass.h"
#include "PaulEngine/Scene/Scene.h"
namespace PaulEngine
{
	class FrameRenderer
	{
	public:
		FrameRenderer(std::vector<RenderPass> orderedPasses = {}) : m_OrderedRenderPasses(orderedPasses) {}

		void RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera);

	private:
		std::vector<RenderPass> m_OrderedRenderPasses;
	};
}