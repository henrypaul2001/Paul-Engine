#pragma once
#include "RenderPass.h"
#include "PaulEngine/Scene/Scene.h"
namespace PaulEngine
{
	class FrameRenderer
	{
	public:
		FrameRenderer() {}

		void RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera, glm::mat4 cameraWorldTransform);

		template <typename T, typename... Args>
		bool AddRenderResource(const char* uniqueName, Args&&... args)
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

		bool AddRenderPass(RenderPass renderPass, Ref<Framebuffer> targetFramebuffer = nullptr, std::vector<const char*> inputBindings = {});

		template <typename T>
		T* GetRenderResource(const char* resourceName)
		{
			IRenderComponent* component = GetRenderResource(resourceName);
			if (component)
			{
				T* casted_component = dynamic_cast<T*>(component);
				if (!casted_component) { PE_CORE_ERROR("Error casting render component '{0}'. nullptr returned", resourceName); }
				return casted_component;
			}
			return nullptr;
		}

		IRenderComponent* GetRenderResource(const char* resourceName)
		{
			auto it = m_RenderResources.find(resourceName);
			if (it != m_RenderResources.end())
			{
				return m_RenderResources[resourceName].get();
			}
			PE_CORE_WARN("Unknown render resource '{0}'", resourceName);
			return nullptr;
		}

	private:
		std::vector<RenderPass> m_OrderedRenderPasses;
		std::unordered_map<const char*, Scope<IRenderComponent>> m_RenderResources;
		std::unordered_map<UUID, Ref<Framebuffer>> m_FramebufferMap;
		std::unordered_map<UUID, std::vector<IRenderComponent*>> m_InputMap;
	};
}