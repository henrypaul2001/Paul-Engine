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
		using OnRenderFunc = std::function<void(RenderPassContext&)>;

		RenderPass(RenderPassInputs inputs, Ref<Framebuffer> targetFramebuffer, OnRenderFunc renderFunc) :
			m_Context({ inputs.SourceTextures, inputs.SourceFramebuffer, targetFramebuffer }), m_RenderFunc(renderFunc) {}

		void OnRender() { m_RenderFunc(m_Context); }

	private:
		OnRenderFunc m_RenderFunc;

		RenderPassContext m_Context;

		friend class FrameRenderer;
	};
}