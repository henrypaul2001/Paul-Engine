#pragma once
#include "Renderer.h"
#include "Renderer2D.h"

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

	struct RenderPassProfile
	{
		uint32_t DrawCalls3D = 0;
		uint32_t MeshCount = 0;
		uint32_t UniquePipelines = 0;

		uint32_t DrawCalls2D = 0;
		uint32_t QuadCount2D = 0;
		uint32_t CircleCount2D = 0;
		uint32_t LineCount2D = 0;
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

		void OnRender(RenderPassContext context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			m_RenderFunc(context, targetFramebuffer, inputs);

			m_Profile = RenderPassProfile();

			Renderer2D::Statistics stats2D = Renderer2D::GetStats();
			m_Profile.DrawCalls2D = stats2D.DrawCalls;
			m_Profile.QuadCount2D = stats2D.QuadCount;
			m_Profile.CircleCount2D = stats2D.CircleCount;
			m_Profile.LineCount2D = stats2D.LineCount;
			Renderer2D::ResetStats();

			Renderer::Statistics stats3D = Renderer::GetStats();
			m_Profile.DrawCalls3D = stats3D.DrawCalls;
			m_Profile.MeshCount = stats3D.MeshCount;
			m_Profile.UniquePipelines = stats3D.PipelineCount;
			Renderer::ResetStats();
		}

		const std::vector<RenderComponentType>& GetInputTypes() const { return m_InputTypes; }
		const UUID& GetRenderID() const { return m_RenderPassID; }

		const RenderPassProfile GetProfile() const { return m_Profile; }

		friend class FrameRenderer;
	private:
		RenderPassProfile m_Profile;
		UUID m_RenderPassID;
		OnRenderFunc m_RenderFunc;
	
		std::vector<RenderComponentType> m_InputTypes;
	};
}