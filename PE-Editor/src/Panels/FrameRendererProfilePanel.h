#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Core/Log.h"
#include "PaulEngine/Renderer/FrameRenderer.h"

namespace PaulEngine
{
	class FrameRendererProfilePanel
	{
	public:
		FrameRendererProfilePanel() {}
		FrameRendererProfilePanel(Ref<const FrameRenderer> context);

		void SetContext(Ref<const FrameRenderer> context);
		
		void OnImGuiRender();

	private:
		static void DrawPassProfile(const std::string& name, const RenderPassProfile& profile);

		Ref<const FrameRenderer> m_Context;
	};
}