#include "pepch.h"
#include "UniformBuffer.h"

#include "PaulEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace PaulEngine
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding, BufferUsage usage)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLUniformBuffer>(size, binding, usage);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
}