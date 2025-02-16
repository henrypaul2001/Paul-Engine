#include "pepch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace PaulEngine {
	VertexArray::~VertexArray() {}

	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::None:		PE_CORE_ASSERT(false, "RendererAPI::None is not supported"); return nullptr;
			case RendererAPI::OpenGL:	return new OpenGLVertexArray();
			case RendererAPI::Direct3D: PE_CORE_ASSERT(false, "RendererAPI::Direct3D is not supported"); return nullptr;
			case RendererAPI::Vulkan:	PE_CORE_ASSERT(false, "RendererAPI::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}