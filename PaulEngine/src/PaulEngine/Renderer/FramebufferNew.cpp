#include "pepch.h"
#include "FramebufferNew.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLFramebufferNew.h"

namespace PaulEngine
{
	Ref<FramebufferNew> FramebufferNew::Create(const FramebufferSpecification& spec, std::vector<Ref<FramebufferAttachment>> colourAttachments, Ref<FramebufferAttachment> depthAttachment)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLFramebufferNew>(spec, colourAttachments, depthAttachment);
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
}