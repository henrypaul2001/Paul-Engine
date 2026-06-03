#include "pepch.h"
#include "UniformBufferStorage.h"

#include "PaulEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBufferStorage.h"

namespace PaulEngine
{
	Ref<UniformBufferStorage> UniformBufferStorage::Create(std::vector<BufferElement> layout, uint32_t binding)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLUniformBufferStorage>(layout, binding);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
}