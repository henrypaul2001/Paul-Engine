#pragma once
#include "PaulEngine/Renderer/Resource/Framebuffer.h"
namespace PaulEngine
{
	class OpenGLFramebufferTexture2DAttachment : public FramebufferTexture2DAttachment
	{
	public:
		OpenGLFramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle textureHandle) : FramebufferTexture2DAttachment(attachPoint, textureHandle) {}
		OpenGLFramebufferTexture2DAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, bool persistentAsset = false) : FramebufferTexture2DAttachment(attachPoint, textureSpec, persistentAsset) {}

		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebufferTexture2DArrayAttachment : public FramebufferTexture2DArrayAttachment
	{
	public:
		OpenGLFramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle textureArrayHandle) : FramebufferTexture2DArrayAttachment(attachPoint, textureArrayHandle) {}
		OpenGLFramebufferTexture2DArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers, bool persistentAsset = false) : FramebufferTexture2DArrayAttachment(attachPoint, textureSpec, layers, persistentAsset) {}

		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebufferTextureCubemapAttachment : public FramebufferTextureCubemapAttachment
	{
	public:
		OpenGLFramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapHandle) :FramebufferTextureCubemapAttachment(attachPoint, cubemapHandle) {}
		OpenGLFramebufferTextureCubemapAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData, bool persistentAsset = false) : FramebufferTextureCubemapAttachment(attachPoint, textureSpec, faceData, persistentAsset) {}
	
		virtual void BindToFramebuffer(const Framebuffer* targetFramebuffer) override;
	};

	class OpenGLFramebufferTextureCubemapArrayAttachment : public FramebufferTextureCubemapArrayAttachment
	{
	public:
		OpenGLFramebufferTextureCubemapArrayAttachment(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapHandle) :FramebufferTextureCubemapArrayAttachment(attachPoint, cubemapHandle) {}
		OpenGLFramebufferTextureCubemapArrayAttachment(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<std::vector<Buffer>> faceDataLayers, bool persistentAsset = false) : FramebufferTextureCubemapArrayAttachment(attachPoint, textureSpec, faceDataLayers, persistentAsset) {}

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

		virtual void SetDrawBuffers(std::vector<FramebufferAttachmentPoint> colourBuffers) override;
		virtual void SetDrawBuffers() override;

		uint32_t GetRendererID() const { return m_RendererID; }

		virtual bool operator ==(const Framebuffer* other) const override;
		virtual bool operator !=(const Framebuffer* other) const override;

	private:
		uint32_t m_RendererID;
		FramebufferSpecification m_Spec;

		std::map<FramebufferAttachmentPoint, Ref<FramebufferAttachment>> m_ColourAttachmentMap;
		Ref<FramebufferAttachment> m_DepthAttachment = nullptr;
	};
}