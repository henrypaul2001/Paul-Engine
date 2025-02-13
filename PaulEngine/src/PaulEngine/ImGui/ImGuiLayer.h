#pragma once
#include "PaulEngine/Layer.h"
#include <PaulEngine/Events/ApplicationEvent.h>
#include <PaulEngine/Events/MouseEvent.h>
#include <PaulEngine/Events/KeyEvent.h>

struct GLFWwindow;

namespace PaulEngine {
	class PAUL_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};
}