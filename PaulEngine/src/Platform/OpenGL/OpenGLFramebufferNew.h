#pragma once
#include "PaulEngine/Renderer/FramebufferNew.h"
namespace PaulEngine
{
	class OpenGLFramebufferNew : public FramebufferNew
	{
	public:
		OpenGLFramebufferNew(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr);
		virtual ~OpenGLFramebufferNew();

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Spec; }

		virtual Ref<FramebufferAttachment> GetAttachment(FramebufferAttachmentPoint attachPoint) override;
		virtual Ref<FramebufferAttachment> GetDepthAttachment() override { return m_DepthAttachment; }

		virtual bool AddColourAttachment(Ref<FramebufferAttachment> colourAttachment) override;
		virtual bool SetDepthAttachment(Ref<FramebufferAttachment> depthAttachment) override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(const uint32_t width, const uint32_t height) override;
		virtual int ReadPixel(FramebufferAttachmentPoint attachPoint, int x, int y) override; // If there are no colour buffers and you want to read the depth buffer, glReadBuffer() must be GL_NONE with the data type set to GL_DEPTH_COMPONENT

		virtual bool operator ==(const FramebufferNew* other) const override;
		virtual bool operator !=(const FramebufferNew* other) const override;

	private:
		uint32_t m_RendererID;
		FramebufferSpecification m_Spec;

		std::unordered_map<FramebufferAttachmentPoint, Ref<FramebufferAttachment>> m_ColourAttachmentMap;
		Ref<FramebufferAttachment> m_DepthAttachment = nullptr;
	};
}