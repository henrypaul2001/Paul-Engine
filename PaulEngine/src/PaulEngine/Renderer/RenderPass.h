#pragma once
#include "Framebuffer.h"
#include "Texture.h"

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
		using OnStartFunc = std::function<void(RenderPassContext&)>;
		using OnRenderFunc = std::function<void(RenderPassContext&)>;
		using OnEndFunc = std::function<void(RenderPassContext&)>;

		RenderPass(RenderPassInputs inputs, Ref<Framebuffer> targetFramebuffer, OnRenderFunc renderFunc, OnStartFunc startFunc = [](RenderPassContext&) {}, OnEndFunc endFunc = [](RenderPassContext&) {}) :
			m_Context({ inputs.SourceTextures, inputs.SourceFramebuffer, targetFramebuffer }),
			m_StartFunc(startFunc), m_RenderFunc(renderFunc), m_EndFunc(endFunc) {}

		void OnStart() { m_StartFunc(m_Context); }
		void OnRender() { m_RenderFunc(m_Context); }
		void OnEnd() { m_EndFunc(m_Context); }

	private:
		OnStartFunc m_StartFunc;
		OnRenderFunc m_RenderFunc;
		OnEndFunc m_EndFunc;

		RenderPassContext m_Context;

		friend class FrameRenderer;
	};
}