#pragma once
#include "PaulEngine/Core/Layer.h"
#include <PaulEngine/Events/ApplicationEvent.h>
#include <PaulEngine/Events/MouseEvent.h>
#include <PaulEngine/Events/KeyEvent.h>

struct GLFWwindow;

namespace PaulEngine {
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void SetBlockEvents(bool block) { m_BlockEvents = block; }

	private:
		float m_Time = 0.0f;
		bool m_BlockEvents = true;
	};
}