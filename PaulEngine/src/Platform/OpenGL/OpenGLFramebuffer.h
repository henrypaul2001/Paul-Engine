#pragma once
#include "PaulEngine/Renderer/Framebuffer.h"

namespace PaulEngine {
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Regenerate();

		virtual const FramebufferSpecification& GetSpecification() const override { return m_FramebufferSpec; }

		virtual uint32_t GetColourAttachmentID() const override { return m_ColourAttachment; }

		virtual void Bind() override;
		virtual void Unbind() override;
	private:
		uint32_t m_RendererID, m_ColourAttachment, m_DepthAttachment;
		FramebufferSpecification m_FramebufferSpec;
	};
}