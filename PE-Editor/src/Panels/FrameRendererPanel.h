#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Core/Log.h"
#include "PaulEngine/Renderer/FrameRenderer.h"

namespace PaulEngine
{
	class FrameRendererPanel
	{
	public:
		FrameRendererPanel() {}
		FrameRendererPanel(const Ref<FrameRenderer>& context);

		void SetContext(const Ref<FrameRenderer>& context);

		void OnImGuiRender();

	private:
		Ref<FrameRenderer> m_Context;
	};
}