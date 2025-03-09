#pragma once
#include "PaulEngine/Renderer/Framebuffer.h"

namespace PaulEngine {
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		virtual void Resize(const uint32_t width, const uint32_t height) override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Spec; }

		virtual uint32_t GetColourAttachmentID(uint32_t index = 0) const override { PE_CORE_ASSERT(index < m_ColourAttachments.size(), "Index out of range"); return m_ColourAttachments[index]; }
		virtual void ClearColourAttachment(uint32_t index, const int value) override;

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		void Regenerate();
		void Deallocate();

		uint32_t m_RendererID;
		FramebufferSpecification m_Spec;

		std::vector<FramebufferTextureSpecification> m_ColourAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureSpecification(FramebufferTextureFormat::None);

		std::vector<uint32_t> m_ColourAttachments;
		uint32_t m_DepthAttachment;
	};
}