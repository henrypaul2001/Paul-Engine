#pragma once
#include "Core.h"
#include "Events/ApplicationEvent.h"
#include "Window.h"
#include "LayerStack.h"
#include "ImGui/ImGuiLayer.h"

#include "PaulEngine/Renderer/Shader.h"

namespace PaulEngine {
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
		std::unique_ptr<Shader> m_Shader;

		static Application* s_Instance;
	};

	// To be defined by client
	Application* CreateApplication();
}