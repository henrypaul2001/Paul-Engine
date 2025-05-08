#pragma once
#include "PaulEngine/Renderer/RenderComponent.h"
#include "PaulEngine/Scene/Scene.h"

#include <functional>
namespace PaulEngine
{
	struct RenderPassInputs
	{
		std::vector<Ref<Texture>> SourceTextures = {};
		Ref<Framebuffer> SourceFramebuffer = nullptr;
	};

	struct RenderPassContext
	{
		std::vector<Ref<Texture>> SourceTextures;
		Ref<Framebuffer> SourceFramebuffer;
		Ref<Framebuffer> TargetFramebuffer;
	};

	class RenderPass
	{
	public:
		using OnRenderFunc = std::function<void(RenderPassContext&, Ref<Scene>, Ref<Camera> activeCamera)>;

		RenderPass(RenderPassInputs inputs, Ref<Framebuffer> targetFramebuffer, OnRenderFunc renderFunc) :
			m_Context({ inputs.SourceTextures, inputs.SourceFramebuffer, targetFramebuffer }), m_RenderFunc(renderFunc) {}

		void OnRender(Ref<Scene> sceneContext, Ref<Camera> activeCamera) { m_RenderFunc(m_Context, sceneContext, activeCamera); }

	private:
		OnRenderFunc m_RenderFunc;

		RenderPassContext m_Context;

		friend class FrameRenderer;
	};

	class RenderPassNew
	{
	public:
		using OnRenderFunc = std::function<void(Ref<Framebuffer>, std::vector<IRenderComponent*>)>;

		RenderPassNew(std::vector<RenderComponentType> inputTypes) {}

		void OnRender(Ref<Framebuffer> targetFramebuffer, std::vector<IRenderComponent*> inputs) {}

		const std::vector<RenderComponentType>& GetInputTypes() const { return m_InputTypes; }

		friend class FrameRendererNew;
	private:
		OnRenderFunc m_RenderFunc;
	
		std::vector<RenderComponentType> m_InputTypes;
	};
}