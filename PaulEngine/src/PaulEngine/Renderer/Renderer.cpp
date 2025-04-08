#include "pepch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"

namespace PaulEngine {
	void Renderer::Init()
	{
		PE_PROFILE_FUNCTION();
		RenderCommand::Init();
		Renderer2D::Init();
	}
}