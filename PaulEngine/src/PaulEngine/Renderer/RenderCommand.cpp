#include "pepch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRenderAPI.h"

namespace PaulEngine {
	RenderAPI* RenderCommand::s_RenderAPI = new OpenGLRenderAPI();
}