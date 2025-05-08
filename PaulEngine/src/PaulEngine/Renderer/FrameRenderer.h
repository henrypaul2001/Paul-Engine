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

	class FrameRendererNew
	{
	public:
		FrameRendererNew(std::vector<RenderPassNew> orderedPasses = {}) : m_OrderedRenderPasses(orderedPasses) {}

		void RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera);

	private:
		std::vector<RenderPassNew> m_OrderedRenderPasses;
		std::unordered_map<const char*, Scope<IRenderComponent>> m_RenderResources;

	};
}