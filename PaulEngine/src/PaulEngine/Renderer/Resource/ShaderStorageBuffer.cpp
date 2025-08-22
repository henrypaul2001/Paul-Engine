#include "pepch.h"
#include "ShaderStorageBuffer.h"

#include "PaulEngine/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShaderStorageBuffer.h"

namespace PaulEngine
{
	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(size_t size, uint32_t binding, const StorageBufferMapping mapping, const bool dynamicStorage)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLShaderStorageBuffer>(size, binding, mapping, dynamicStorage);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
}