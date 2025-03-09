#include "pepch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace PaulEngine {

	namespace FramebufferUtils
	{
		static GLenum TextureTarget(bool multisampled) {
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* out_ID, uint32_t count) {
			glCreateTextures(TextureTarget(multisampled), count, out_ID);
		}

		static void BindTexture(bool multisampled, uint32_t id) {
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColourTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index) {
			bool multisampled = (samples > 1);
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height) {
			bool multisampled = (samples > 1);
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format) {
			switch (format) {
				case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
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
			FramebufferUtils::CreateTextures(multisample, &m_ColourAttachments[0], m_ColourAttachments.size());

			for (size_t i = 0; i < m_ColourAttachmentSpecs.size(); i++) {
				FramebufferUtils::BindTexture(multisample, m_ColourAttachments[i]);
				switch (m_ColourAttachmentSpecs[i].TextureFormat) {
					case FramebufferTextureFormat::RGBA8:
						FramebufferUtils::AttachColourTexture(m_ColourAttachments[i], m_Spec.Samples, GL_RGBA8, GL_RGBA, m_Spec.Width, m_Spec.Height, i);
						break;
					case FramebufferTextureFormat::RED_INTEGER:
						FramebufferUtils::AttachColourTexture(m_ColourAttachments[i], m_Spec.Samples, GL_R32I, GL_RED_INTEGER, m_Spec.Width, m_Spec.Height, i);
						break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) {
			FramebufferUtils::CreateTextures(multisample, &m_DepthAttachment, 1);
			FramebufferUtils::BindTexture(multisample, m_DepthAttachment);
			switch (m_DepthAttachmentSpec.TextureFormat) {
				case FramebufferTextureFormat::DEPTH24STENCIL8:
					FramebufferUtils::AttachDepthTexture(m_DepthAttachment, m_Spec.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Spec.Width, m_Spec.Height);
					break;
			}
		}

		if (m_ColourAttachments.size() > 1) {
			PE_CORE_ASSERT(m_ColourAttachments.size() <= 4, "Maximum of 4 colour attachments");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColourAttachments.size(), buffers);
		}
		else if (m_ColourAttachments.size() == 0) {
			glDrawBuffer(GL_NONE);
		}

		PE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

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
}