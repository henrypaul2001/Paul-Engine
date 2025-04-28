#include "pepch.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>

namespace PaulEngine {

	namespace FramebufferUtils
	{
		static GLenum TextureTarget(bool multisampled) {
			return OpenGLTexture2D::TextureTarget(multisampled);
		}

		static GLenum FramebufferTextureFormatToGLFormat(FramebufferTextureFormat format)
		{
			switch (format) {
				case FramebufferTextureFormat::RGBA8: return GL_RGBA8;
				case FramebufferTextureFormat::RED_INTEGER: return GL_R32I;
				case FramebufferTextureFormat::DEPTH16: return GL_DEPTH_COMPONENT16;
				case FramebufferTextureFormat::DEPTH24: return GL_DEPTH_COMPONENT24;
				case FramebufferTextureFormat::DEPTH32: return GL_DEPTH_COMPONENT32;
				case FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
			}

			PE_CORE_ASSERT(false, "Undefined FramebufferTextureFormat translation");
			return 0;
		}

		static GLenum FramebufferTextureFormatToGLDataFormat(FramebufferTextureFormat format)
		{
			switch (format) {
				case FramebufferTextureFormat::RGBA8: return GL_RGBA;
				case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
				case FramebufferTextureFormat::DEPTH16: return GL_DEPTH_ATTACHMENT;
				case FramebufferTextureFormat::DEPTH24: return GL_DEPTH_ATTACHMENT;
				case FramebufferTextureFormat::DEPTH32: return GL_DEPTH_ATTACHMENT;
				case FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL_ATTACHMENT;
			}

			PE_CORE_ASSERT(false, "Undefined FramebufferTextureFormat translation");
			return 0;
		}

		static void AttachColourTexture(uint32_t id, int samples, uint32_t width, uint32_t height, int index, FramebufferTextureSpecification spec) {
			bool multisampled = (samples > 1);
			GLenum format = FramebufferTextureFormatToGLDataFormat(spec.TextureFormat);
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, FramebufferTextureFormatToGLFormat(spec.TextureFormat), width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(spec.MinFilter));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(spec.MagFilter));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(spec.Wrap_R));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(spec.Wrap_S));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(spec.Wrap_T));
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &spec.Border[0]);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, uint32_t width, uint32_t height, FramebufferTextureSpecification spec) {
			bool multisampled = (samples > 1);
			GLenum format = FramebufferTextureFormatToGLDataFormat(spec.TextureFormat);
			GLenum internalFormat = FramebufferTextureFormatToGLFormat(spec.TextureFormat);
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else {
				glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGLTextureUtils::MinFilterToGLMinFilter(spec.MinFilter));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGLTextureUtils::MagFilterToGLMagFilter(spec.MagFilter));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, OpenGLTextureUtils::ImageWrapToGLWrap(spec.Wrap_R));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTextureUtils::ImageWrapToGLWrap(spec.Wrap_S));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTextureUtils::ImageWrapToGLWrap(spec.Wrap_T));
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &spec.Border[0]);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, format, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format) {
			switch (format) {
				case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
				case FramebufferTextureFormat::DEPTH16: return true;
				case FramebufferTextureFormat::DEPTH24: return true;
				case FramebufferTextureFormat::DEPTH32: return true;
			}
			return false;
		}

		static GLenum PETexFormatToGLTexFormat(FramebufferTextureFormat format) {
			switch (format) {
			case FramebufferTextureFormat::RGBA8:
				return GL_RGBA8;
			case FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
			}

			PE_CORE_ASSERT(false, "Undefined texture format translation");
			return 0;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_Spec(spec)
	{
		for (auto textureAttachment : m_Spec.Attachments.TextureAttachments) {
			if (!FramebufferUtils::IsDepthFormat(textureAttachment.TextureFormat)) {
				m_ColourAttachmentSpecs.emplace_back(textureAttachment);
			}
			else {
				m_DepthAttachmentSpec = textureAttachment;
			}
		}
		
		Regenerate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		Deallocate();
	}

	void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height)
	{
		if (width == 0 || height == 0) {
			PE_CORE_WARN("Attempt to resize framebuffer to invalid dimensions: {0}, {1}", width, height);
			return;
		}

		m_Spec.Width = width;
		m_Spec.Height = height;

		Regenerate();
	}

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		PE_CORE_ASSERT(attachmentIndex < m_ColourAttachments.size(), "Framebuffer attachment index out of bounds");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

	void OpenGLFramebuffer::Regenerate()
	{
		PE_PROFILE_FUNCTION();

		if (m_RendererID) {
			Deallocate();
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// Attachments
		bool multisample = (m_Spec.Samples > 1);
		if (m_ColourAttachmentSpecs.size() > 0) {
			m_ColourAttachments.resize(m_ColourAttachmentSpecs.size());
			OpenGLTexture2D::CreateTextures(multisample, &m_ColourAttachments[0], m_ColourAttachments.size());

			for (size_t i = 0; i < m_ColourAttachmentSpecs.size(); i++) {
				OpenGLTexture2D::BindTexture(0, m_ColourAttachments[i]);
				FramebufferUtils::AttachColourTexture(m_ColourAttachments[i], m_Spec.Samples, m_Spec.Width, m_Spec.Height, i, m_ColourAttachmentSpecs[i]);
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) {
			OpenGLTexture2D::CreateTextures(multisample, &m_DepthAttachment, 1);
			OpenGLTexture2D::BindTexture(0, m_DepthAttachment);
			FramebufferUtils::AttachDepthTexture(m_DepthAttachment, m_Spec.Samples, m_Spec.Width, m_Spec.Height, m_DepthAttachmentSpec);
		}

		if (m_ColourAttachments.size() > 1) {
			PE_CORE_ASSERT(m_ColourAttachments.size() <= 4, "Maximum of 4 colour attachments");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColourAttachments.size(), buffers);
		}
		else if (m_ColourAttachments.size() == 0) {
			glDrawBuffer(GL_NONE);
		}

		// error check
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			PE_CORE_ERROR("Framebuffer incomplete. Status: {0}", status);
			PE_CORE_ASSERT(false, "Framebuffer is incomplete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Deallocate()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		if (m_ColourAttachments.size() > 0) {
			glDeleteTextures(m_ColourAttachments.size(), &m_ColourAttachments[0]);
			m_ColourAttachments.clear();
		}
		if (m_DepthAttachment) {
			glDeleteTextures(1, &m_DepthAttachment);
		}
	}

	void OpenGLFramebuffer::ClearColourAttachment(uint32_t index, const int value)
	{
		PE_CORE_ASSERT(index < m_ColourAttachments.size(), "Colour attachment index out of bounds!");
		glClearTexImage(m_ColourAttachments[index], 0, FramebufferUtils::PETexFormatToGLTexFormat(m_ColourAttachmentSpecs[index].TextureFormat), GL_INT, &value);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
}