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
	Ref<FramebufferTexture2DAttachment> FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint attachPoint, AssetHandle textureHandle)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DAttachment>(attachPoint, textureHandle);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTexture2DAttachment> FramebufferTexture2DAttachment::Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, bool persistentAsset)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DAttachment>(attachPoint, textureSpec, persistentAsset);
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
	Ref<FramebufferTexture2DArrayAttachment> FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint attachPoint, AssetHandle textureArrayHandle)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DArrayAttachment>(attachPoint, textureArrayHandle);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTexture2DArrayAttachment> FramebufferTexture2DArrayAttachment::Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> layers, bool persistentAsset)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTexture2DArrayAttachment>(attachPoint, textureSpec, layers, persistentAsset);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTextureCubemapAttachment> FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapHandle)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTextureCubemapAttachment>(attachPoint, cubemapHandle);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTextureCubemapAttachment> FramebufferTextureCubemapAttachment::Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<Buffer> faceData, bool persistentAsset)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTextureCubemapAttachment>(attachPoint, textureSpec, faceData, persistentAsset);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTextureCubemapArrayAttachment> FramebufferTextureCubemapArrayAttachment::Create(FramebufferAttachmentPoint attachPoint, AssetHandle cubemapArrayHandle)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTextureCubemapArrayAttachment>(attachPoint, cubemapArrayHandle);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
	Ref<FramebufferTextureCubemapArrayAttachment> FramebufferTextureCubemapArrayAttachment::Create(FramebufferAttachmentPoint attachPoint, TextureSpecification textureSpec, std::vector<std::vector<Buffer>> faceDataLayers, bool persistentAsset)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferTextureCubemapArrayAttachment>(attachPoint, textureSpec, faceDataLayers, persistentAsset);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}

	void FramebufferTexture2DAttachment::Resize(const uint32_t width, const uint32_t height)
	{
		AssetManager::GetAsset<Texture2D>(m_TextureHandle)->Resize(width, height);
	}

	void FramebufferTexture2DArrayAttachment::Resize(const uint32_t width, const uint32_t height)
	{
		AssetManager::GetAsset<Texture2DArray>(m_TextureArrayHandle)->Resize(width, height);
	}

	void FramebufferTextureCubemapAttachment::Resize(const uint32_t width, const uint32_t height)
	{
		AssetManager::GetAsset<TextureCubemap>(m_CubemapHandle)->Resize(width, height);
	}

	void FramebufferTextureCubemapArrayAttachment::Resize(const uint32_t width, const uint32_t height)
	{
		AssetManager::GetAsset<TextureCubemapArray>(m_CubemapHandle)->Resize(width, height);
	}
}