#include "pepch.h"
#include "Framebuffer.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace PaulEngine
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments, Ref<FramebufferAttachment> depthAttachment)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebuffer>(spec, colourAttachments, depthAttachment);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTexture2DAttachment> FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint attachPoint, Ref<Texture2D> texture)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DAttachment>(attachPoint, texture);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTexture2DAttachment> FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DAttachment>(attachPoint, textureSpec);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferRenderbufferAttachment> FramebufferRenderbufferAttachment::Create(FramebufferAttachmentPoint attachPoint)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferRenderbufferAttachment>(attachPoint);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTexture2DArrayAttachment> FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint attachPoint, Ref<Texture2DArray> textureArray)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DArrayAttachment>(attachPoint, textureArray);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}

	Ref<FramebufferTexture2DArrayAttachment> FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DArrayAttachment>(attachPoint, textureSpec, layers);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}

	void FramebufferTexture2DAttachment::Resize(const uint32_t width, const uint32_t height)
	{
		TextureSpecification textureSpec = m_Texture->GetSpecification();
		if (textureSpec.Width != width || textureSpec.Height != height) {
			textureSpec.Width = width;
			textureSpec.Height = height;
			m_Texture = Texture2D::Create(textureSpec);
		}
	}

	void FramebufferTexture2DArrayAttachment::Resize(const uint32_t width, const uint32_t height)
	{
		TextureSpecification textureSpec = m_TextureArray->GetSpecification();
		if (textureSpec.Width != width || textureSpec.Height != height) {
			textureSpec.Width = width;
			textureSpec.Height = height;
			uint8_t numLayers = m_TextureArray->GetNumLayers();
			m_TextureArray = Texture2DArray::Create(textureSpec, std::vector<Buffer>(numLayers));
		}
	}

}