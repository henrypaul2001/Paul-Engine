#pragma once
#include "Core.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"

namespace PaulEngine {
	class PAUL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();

	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined by client
	Application* CreateApplication();
}