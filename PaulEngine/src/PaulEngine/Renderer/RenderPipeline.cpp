#include "pepch.h"
#include "Renderer.h"
#include "RenderPipeline.h"

#include "Platform/OpenGL/OpenGLRenderPipeline.h"

namespace PaulEngine
{
	Ref<RenderPipeline> RenderPipeline::Create(FaceCulling cullState, DepthState depthState, BlendState blendState, AssetHandle material)
	{
		PE_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
			case RenderAPI::API::None:		PE_CORE_ASSERT(false, "RenderAPI::API::None is not supported"); return nullptr;
			case RenderAPI::API::OpenGL:	return CreateRef<OpenGLRenderPipeline>(cullState, depthState, blendState, material);
			case RenderAPI::API::Direct3D:  PE_CORE_ASSERT(false, "RenderAPI::API::Direct3D is not supported"); return nullptr;
			case RenderAPI::API::Vulkan:	PE_CORE_ASSERT(false, "RenderAPI::API::Vulkan is not supported"); return nullptr;
		}

		PE_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
}