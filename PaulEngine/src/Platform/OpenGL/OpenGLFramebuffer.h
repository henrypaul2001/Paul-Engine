#pragma once
#include "PaulEngine/Renderer/Framebuffer.h"
namespace PaulEngine
{
	class OpenGLFramebufferTexture2DAttachment : public FramebufferTexture2DAttachment
	{
	public:
		OpenGLFramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, Ref<Texture2D> texture) : FramebufferTexture2DAttachment(attachPoint, texture) {}
		OpenGLFramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec) : FramebufferTexture2DAttachment(attachPoint, textureSpec) {}

		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebufferTexture2DArrayAttachment : public FramebufferTexture2DArrayAttachment
	{
	public:
		OpenGLFramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, Ref<Texture2DArray> textureArray) : FramebufferTexture2DArrayAttachment(attachPoint, textureArray) {}
		OpenGLFramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers) : FramebufferTexture2DArrayAttachment(attachPoint, textureSpec, layers) {}

		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebufferTextureCubemapAttachment : public FramebufferTextureCubemapAttachment
	{
	public:
		OpenGLFramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, Ref<TextureCubemap> cubemap) :FramebufferTextureCubemapAttachment(attachPoint, cubemap) {}
		OpenGLFramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData) : FramebufferTextureCubemapAttachment(attachPoint, textureSpec, faceData) {}
	
		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebufferRenderbufferAttachment : public FramebufferRenderbufferAttachment
	{
	public:
		OpenGLFramebufferRenderbufferAttachment(FramebufferAttachmentPoint attachPoint) : FramebufferRenderbufferAttachment(attachPoint) {}

		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments = {}, Ref<FramebufferAttachment> depthAttachment = nullptr);
		virtual ~OpenGLFramebuffer();

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Spec; }

		virtual Ref<FramebufferAttachment> GetAttachment(FramebufferAttachmentPoint attachPoint) override;
		virtual Ref<FramebufferAttachment> GetDepthAttachment() override { return m_DepthAttachment; }

		virtual bool AddColourAttachment(Ref<FramebufferAttachment> colourAttachment) override;
		virtual bool SetDepthAttachment(Ref<FramebufferAttachment> depthAttachment) override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(const uint32_t width, const uint32_t height) override;
		virtual int ReadPixel(FramebufferAttachmentPoint attachPoint, int x, int y) override; // If there are no colour buffers and you want to read the depth buffer, glReadBuffer() must be GL_NONE with the data type set to GL_DEPTH_COMPONENT

		uint32_t GetRendererID() const { return m_RendererID; }

		virtual bool operator ==(const Framebuffer* other) const override;
		virtual bool operator !=(const Framebuffer* other) const override;

	private:
		uint32_t m_RendererID;
		FramebufferSpecification m_Spec;

		std::unordered_map<FramebufferAttachmentPoint, Ref<FramebufferAttachment>> m_ColourAttachmentMap;
		Ref<FramebufferAttachment> m_DepthAttachment = nullptr;
	};
}