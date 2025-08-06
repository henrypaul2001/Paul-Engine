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
		
		RenderPass(std::vector<RenderComponentType> inputTypes, OnRenderFunc renderFunc, const std::string& debugName = "numbered_render_pass") : m_InputTypes(inputTypes), m_RenderFunc(renderFunc), m_DebugName(debugName) {}

		RenderPassProfile OnRender(RenderPassContext context, Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {
			m_RenderFunc(context, targetFramebuffer, inputs);

			RenderPassProfile profile;

			Renderer2D::Statistics stats2D = Renderer2D::GetStats();
			profile.DrawCalls2D = stats2D.DrawCalls;
			profile.QuadCount2D = stats2D.QuadCount;
			profile.CircleCount2D = stats2D.CircleCount;
			profile.LineCount2D = stats2D.LineCount;
			Renderer2D::ResetStats();

			Renderer::Statistics stats3D = Renderer::GetStats();
			profile.DrawCalls3D = stats3D.DrawCalls;
			profile.MeshCount = stats3D.MeshCount;
			profile.UniquePipelines = stats3D.PipelineCount;
			Renderer::ResetStats();

			return profile;
		}

		const std::vector<RenderComponentType>& GetInputTypes() const { return m_InputTypes; }
		const UUID& GetRenderID() const { return m_RenderPassID; }

		const std::string& GetDebugName() const { return m_DebugName; }

		friend class FrameRenderer;
	private:
		std::string m_DebugName;
		UUID m_RenderPassID;
		OnRenderFunc m_RenderFunc;
	
		std::vector<RenderComponentType> m_InputTypes;
	};
}