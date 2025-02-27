#pragma once
#include "Core.h"
#include "PaulEngine/Events/ApplicationEvent.h"
#include "Window.h"
#include "LayerStack.h"
#include "PaulEngine/ImGui/ImGuiLayer.h"

#include "PaulEngine/Core/Timestep.h"

namespace PaulEngine {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();
		void Close();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	// To be defined by client
	Application* CreateApplication();
}