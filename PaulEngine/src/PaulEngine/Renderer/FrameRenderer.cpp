#include "pepch.h"
#include "FrameRenderer.h"

namespace PaulEngine
{
	void FrameRenderer::RenderFrame()
	{
		Ref<Framebuffer> currentTarget = nullptr;
		for (RenderPass& p : m_OrderedRenderPasses) {

			// First check if next render pass uses the same framebuffer as previous pass to avoid state changes
			if (currentTarget.get() && currentTarget.get() != p.m_Context.TargetFramebuffer.get()) {
				if (p.m_Context.TargetFramebuffer) {
					p.m_Context.TargetFramebuffer->Bind();
				}
				else if (currentTarget) {
					currentTarget->Unbind();
				}
			}
			else if (p.m_Context.TargetFramebuffer.get()) {
				p.m_Context.TargetFramebuffer->Bind();
			}
			currentTarget = p.m_Context.TargetFramebuffer;

			p.OnStart();
			p.OnRender();
			p.OnEnd();
		}
	}
}