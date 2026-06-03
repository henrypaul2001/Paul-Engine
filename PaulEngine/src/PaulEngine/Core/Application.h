#pragma once
#include "Core.h"
#include "PaulEngine/Events/ApplicationEvent.h"
#include "Window.h"
#include "LayerStack.h"
#include "PaulEngine/ImGui/ImGuiLayer.h"

#include "PaulEngine/Core/Timestep.h"

namespace PaulEngine {

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;
	
		const char* operator[](int index) const
		{
			PE_CORE_ASSERT(index < Count, "Index out of range");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Paul Engine";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();
		void Close();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		
		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	// To be defined by client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}