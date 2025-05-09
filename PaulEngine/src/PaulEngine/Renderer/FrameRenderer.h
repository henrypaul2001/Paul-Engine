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
		FrameRendererNew() {}

		void RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera);

		template <typename T, typename... Args>
		bool AddRenderResource(const char* uniqueName, Args&... args)
		{
			auto it = m_RenderResources.find(uniqueName);
			if (it != m_RenderResources.end())
			{
				PE_CORE_ERROR("Render resource with name '{0}' already exists in frame renderer", uniqueName);
				return false;
			}
			m_RenderResources[uniqueName] = CreateScope<T>(std::forward<Args>(args)...);
			return true;
		}

		bool AddRenderPass(RenderPassNew renderPass, Ref<Framebuffer> targetFramebuffer = nullptr, std::vector<const char*> inputBindings = {});

	private:
		std::vector<RenderPassNew> m_OrderedRenderPasses;
		std::unordered_map<const char*, Scope<IRenderComponent>> m_RenderResources;
		std::unordered_map<UUID, Ref<Framebuffer>> m_FramebufferMap;
		std::unordered_map<UUID, std::vector<IRenderComponent*>> m_InputMap;
	};
}