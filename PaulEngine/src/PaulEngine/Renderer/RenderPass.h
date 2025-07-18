#pragma once
#include "PaulEngine/Renderer/RenderComponent.h"
#include "PaulEngine/Scene/Scene.h"

#include <functional>
namespace PaulEngine
{
	struct RenderPassParameters
	{
		Ref<Framebuffer> TargetFramebuffer;
		std::vector<IRenderComponent*> InputComponents;
	};

	class RenderPass
	{
	public:
		struct RenderPassContext
		{
			Ref<Scene> ActiveScene;
			Ref<Camera> ActiveCamera;
			glm::mat4 CameraWorldTransform;
		};
		using OnRenderFunc = std::function<void(RenderPassContext&, Ref<Framebuffer>, std::vector<IRenderComponent*>)>; // { scene, camera }, target framebuffer, inputs
		
		RenderPass(std::vector<RenderComponentType> inputTypes, OnRenderFunc renderFunc) : m_InputTypes(inputTypes), m_RenderFunc(renderFunc) {}

		void OnRender(RenderPassContext context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) { m_RenderFunc(context, targetFramebuffer, inputs); }

		const std::vector<RenderComponentType>& GetInputTypes() const { return m_InputTypes; }
		const UUID& GetRenderID() const { return m_RenderPassID; }

		friend class FrameRenderer;
	private:
		UUID m_RenderPassID;
		OnRenderFunc m_RenderFunc;
	
		std::vector<RenderComponentType> m_InputTypes;
	};
}