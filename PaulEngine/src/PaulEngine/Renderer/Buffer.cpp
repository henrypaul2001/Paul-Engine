#include "pepch.h"
#include "Buffer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

#include "Renderer.h"
namespace PaulEngine {
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::None:		PE_CORE_ASSERT(false, "RendererAPI::None is not supported"); return nullptr;
			case RendererAPI::OpenGL:	return new OpenGLVertexBuffer(vertices, size);
			case RendererAPI::Direct3D: PE_CORE_ASSERT(false, "RendererAPI::Direct3D is not supported"); return nullptr;
			case RendererAPI::Vulkan:	PE_CORE_ASSERT(false, "RendererAPI::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::None:		PE_CORE_ASSERT(false, "RendererAPI::None is not supported"); return nullptr;
			case RendererAPI::OpenGL:	return new OpenGLIndexBuffer(indices, count);
			case RendererAPI::Direct3D: PE_CORE_ASSERT(false, "RendererAPI::Direct3D is not supported"); return nullptr;
			case RendererAPI::Vulkan:	PE_CORE_ASSERT(false, "RendererAPI::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}