#pragma once
#include "RenderPass.h"
#include "PaulEngine/Scene/Scene.h"
namespace PaulEngine
{
	class FrameRenderer
	{
	public:
		friend class FrameRendererSerializer;
		using OnEventFunc = std::function<void(Event&, FrameRenderer*)>;
		FrameRenderer(OnEventFunc eventFunc = [](Event& e, FrameRenderer* self) {}) : m_OnEvent(eventFunc) {}

		void RenderFrame(Ref<Scene> sceneContext, Ref<Camera> activeCamera, glm::mat4 cameraWorldTransform);
		void OnEvent(Event& e) { m_OnEvent(e, this); }

		template <typename T, typename... Args>
		bool AddRenderResource(const std::string& uniqueName, bool serialized, Args&&... args)
		{
			auto it = m_RenderResources.find(uniqueName);
			if (it != m_RenderResources.end())
			{
				PE_CORE_ERROR("Render resource with name '{0}' already exists in frame renderer", uniqueName);
				return false;
			}
			m_RenderResources[uniqueName] = CreateScope<T>(std::forward<Args>(args)...);
			if (serialized) { m_SerializedComponentNames.push_back(uniqueName); }
			return true;
		}

		bool AddRenderPass(RenderPass renderPass, Ref<Framebuffer> targetFramebuffer = nullptr, std::vector<std::string> inputBindings = {});

		void SetEventFunc(OnEventFunc func) { m_OnEvent = func; }

		template <typename T>
		T* GetRenderResource(const std::string& resourceName)
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

		IRenderComponent* GetRenderResource(const std::string& resourceName)
		{
			auto it = m_RenderResources.find(resourceName);
			if (it != m_RenderResources.end())
			{
				return m_RenderResources[resourceName].get();
			}
			PE_CORE_WARN("Unknown render resource '{0}'", resourceName);
			return nullptr;
		}

		const std::vector<std::string>& GetSerializedComponentNames() const { return m_SerializedComponentNames; }

		void SaveResourceConfig(const std::filesystem::path& path);
		void LoadResourceConfig(const std::filesystem::path& path);
	private:
		std::vector<RenderPass> m_OrderedRenderPasses;
		std::vector<std::string> m_SerializedComponentNames;
		std::unordered_map<std::string, Scope<IRenderComponent>> m_RenderResources;

		std::unordered_map<UUID, RenderPassParameters> m_ParameterMap;

		OnEventFunc m_OnEvent;
	};
}