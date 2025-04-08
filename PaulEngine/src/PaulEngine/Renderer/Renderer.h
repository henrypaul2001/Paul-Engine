#pragma once
#include "RenderAPI.h"
#include "Shader.h"

namespace PaulEngine{

	class Renderer {
	public:
		static void Init();

		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }
	};
}