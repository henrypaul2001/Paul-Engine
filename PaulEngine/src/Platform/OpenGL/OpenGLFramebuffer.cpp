#include "pepch.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <set>

namespace PaulEngine
{
	namespace OpenGLFramebufferUtils
	{
		static GLenum FramebufferAttachPointToGLenum(FramebufferAttachmentPoint attachPoint)
		{
			switch (attachPoint)
			{
				case FramebufferAttachmentPoint::None: return 0;
				case FramebufferAttachmentPoint::Colour0: return GL_COLOR_ATTACHMENT0;
				case FramebufferAttachmentPoint::Colour1: return GL_COLOR_ATTACHMENT1;
				case FramebufferAttachmentPoint::Colour2: return GL_COLOR_ATTACHMENT2;
				case FramebufferAttachmentPoint::Colour3: return GL_COLOR_ATTACHMENT3;
				case FramebufferAttachmentPoint::Colour4: return GL_COLOR_ATTACHMENT4;
				case FramebufferAttachmentPoint::Colour5: return GL_COLOR_ATTACHMENT5;
				case FramebufferAttachmentPoint::Colour6: return GL_COLOR_ATTACHMENT6;
				case FramebufferAttachmentPoint::Colour7: return GL_COLOR_ATTACHMENT7;
				case FramebufferAttachmentPoint::Depth: return GL_DEPTH_ATTACHMENT;
				case FramebufferAttachmentPoint::Stencil: return GL_STENCIL_ATTACHMENT;
				case FramebufferAttachmentPoint::DepthStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
			}
			PE_CORE_ASSERT(false, "Undefined attachment point translation");
			return 0;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments, Ref<FramebufferAttachment> depthAttachment) : m_RendererID(0), m_Spec(spec)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(colourAttachments.size() <= 8, "Maximum of 8 colour attachments");
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glDrawBuffer(GL_NONE);

		bool multisample = (m_Spec.Samples > 1);
		std::set<FramebufferAttachmentPoint> drawBuffers;
		for (auto attachment : colourAttachments) {
			if (AddColourAttachment(attachment)) {
				drawBuffers.insert(attachment->GetAttachPoint());
			}
		}
		if (drawBuffers.size() > 0) {
			std::vector<GLenum> gl_DrawBuffers;
			for (auto attach : drawBuffers) {
				gl_DrawBuffers.push_back(OpenGLFramebufferUtils::FramebufferAttachPointToGLenum(attach));
			}
			glDrawBuffers(gl_DrawBuffers.size(), &gl_DrawBuffers[0]);
		}
		if (depthAttachment) {
			SetDepthAttachment(depthAttachment);
		}

		// error check
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			PE_CORE_ERROR("Framebuffer incomplete. Status: {0}", status);
			PE_CORE_ASSERT(false, "Framebuffer is incomplete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
	}

	Ref<FramebufferAttachment> OpenGLFramebuffer::GetAttachment(FramebufferAttachmentPoint attachPoint)
	{
		PE_PROFILE_FUNCTION();
		if (FramebufferAttachment::IsDepthAttachment(attachPoint)) {
			if (m_DepthAttachment && m_DepthAttachment->GetAttachPoint() == attachPoint) {
				return m_DepthAttachment;
			}
		}
		else {
			if (m_ColourAttachmentMap.find(attachPoint) != m_ColourAttachmentMap.end()) {
				return m_ColourAttachmentMap.at(attachPoint);
			}
		}
		PE_CORE_ERROR("No attachments were found on framebuffer with attach point {0}", (int)attachPoint);
		return nullptr;
	}

	bool OpenGLFramebuffer::AddColourAttachment(Ref<FramebufferAttachment> colourAttachment)
	{
		if (FramebufferAttachment::IsDepthAttachment(colourAttachment->GetAttachPoint())) {
			PE_CORE_ERROR("Cannot add depth attachment to colour attachments");
			return false;
		}

		m_ColourAttachmentMap[colourAttachment->GetAttachPoint()] = colourAttachment;
		colourAttachment->BindToFramebuffer(this);
		return true;
	}

	bool OpenGLFramebuffer::SetDepthAttachment(Ref<FramebufferAttachment> depthAttachment)
	{
		if (!FramebufferAttachment::IsDepthAttachment(depthAttachment->GetAttachPoint())) {
			PE_CORE_ERROR("Cannot add non depth attachment to depth attachment slot");
			return false;
		}
		m_DepthAttachment = depthAttachment;
		depthAttachment->BindToFramebuffer(this);
		return true;
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height)
	{
		PE_PROFILE_FUNCTION();
		if (width == 0 || height == 0) {
			PE_CORE_WARN("Attempt to resize framebuffer to invalid dimensions: {0}, {1}", width, height);
			return;
		}

		if (width != m_Spec.Width || height != m_Spec.Height)
		{
			m_Spec.Width = width;
			m_Spec.Height = height;
			for (auto colourAttach : m_ColourAttachmentMap) {
				colourAttach.second->Resize(width, height);
				colourAttach.second->BindToFramebuffer(this);
			}
			if (m_DepthAttachment)
			{
				m_DepthAttachment->Resize(width, height);
				m_DepthAttachment->BindToFramebuffer(this);
			}
		}
	}

	// TODO: make this more elegant and provide more data types. Currently fairly hardcoded to suit reading the EntityID value
	int OpenGLFramebuffer::ReadPixel(FramebufferAttachmentPoint attachPoint, int x, int y)
	{
		PE_PROFILE_FUNCTION();
		Ref<FramebufferAttachment> attachment = GetAttachment(attachPoint);
		if (attachment) {
			GLenum glAttachment = OpenGLFramebufferUtils::FramebufferAttachPointToGLenum(attachPoint);

			if (attachment->GetType() == FramebufferAttachmentType::Texture2D) {
				FramebufferTexture2DAttachment* textureAttachment = dynamic_cast<FramebufferTexture2DAttachment*>(attachment.get());
				ImageFormat format = textureAttachment->GetTexture()->GetSpecification().Format;
				GLenum dataFormat = OpenGLTextureUtils::PEImageFormatToGLDataFormat(format);

				if (dataFormat == GL_RED_INTEGER) {
					glReadBuffer(glAttachment);
					int pixelData;
					glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
					return pixelData;
				}
				else {
					PE_CORE_ERROR("Texture format mismatch: requesting {0} from {1}", (int)format, (int)dataFormat);
				}
			}
		}
		return -1;
	}

	bool OpenGLFramebuffer::operator==(const Framebuffer* other) const
	{
		if (!other) { return false; }
		return m_RendererID == dynamic_cast<const OpenGLFramebuffer*>(other)->m_RendererID;
	}

	bool OpenGLFramebuffer::operator!=(const Framebuffer* other) const
	{
		if (!other) { return true; }
		return m_RendererID != dynamic_cast<const OpenGLFramebuffer*>(other)->m_RendererID;
	}

	// Renderbuffer attachment
	void OpenGLFramebufferRenderbufferAttachment::BindToFramebuffer(const Framebuffer* targetFramebuffer)
	{
		PE_CORE_ASSERT(false, "Not yet implemented");
	}

	// Texture2D attachment
	void OpenGLFramebufferTexture2DAttachment::BindToFramebuffer(const Framebuffer* targetFramebuffer)
	{
		const OpenGLFramebuffer* glFramebuffer = dynamic_cast<const OpenGLFramebuffer*>(targetFramebuffer);
		glNamedFramebufferTexture(glFramebuffer->GetRendererID(), OpenGLFramebufferUtils::FramebufferAttachPointToGLenum(m_AttachPoint), m_Texture->GetRendererID(), 0);
	}

	// Texture2DArray attachment
	void OpenGLFramebufferTexture2DArrayAttachment::BindToFramebuffer(const Framebuffer* targetFramebuffer)
	{
		const OpenGLFramebuffer* glFramebuffer = dynamic_cast<const OpenGLFramebuffer*>(targetFramebuffer);
		glNamedFramebufferTextureLayer(glFramebuffer->GetRendererID(), OpenGLFramebufferUtils::FramebufferAttachPointToGLenum(m_AttachPoint), m_TextureArray->GetRendererID(), 0, m_TargetIndex);
	}

	// TextureCubemap attachment
	void OpenGLFramebufferTextureCubemapAttachment::BindToFramebuffer(const Framebuffer* targetFramebuffer)
	{
		const OpenGLFramebuffer* glFramebuffer = dynamic_cast<const OpenGLFramebuffer*>(targetFramebuffer);
		glNamedFramebufferTextureLayer(glFramebuffer->GetRendererID(), OpenGLFramebufferUtils::FramebufferAttachPointToGLenum(m_AttachPoint), m_Cubemap->GetRendererID(), 0, (int)m_TargetFace);
	}
}