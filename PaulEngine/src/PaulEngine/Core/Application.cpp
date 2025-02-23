#include "pepch.h"
#include "Application.h"

#include "PaulEngine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace PaulEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(PE_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{

	}

	void Application::OnEvent(Event& e)
	{
		PE_PROFILE_FUNCTION();
		EventDispatcher dispatcher = EventDispatcher(e);
		dispatcher.DispatchEvent<WindowCloseEvent>(PE_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.DispatchEvent<WindowResizeEvent>(PE_BIND_EVENT_FN(Application::OnWindowResize));

		// Events propagate down the layer stack, starting with overlays until the event is handled or all layers have received the event
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled()) { break; }
		}
	}

	void Application::Run()
	{
		while (m_Running) {
			PE_PROFILE_SCOPE("Frame");

			float time = (float)glfwGetTime(); // Temporary glfw dependency, change to Platform::GetTime() in future
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized) {
				// Update layers
				for (Layer* layer : m_LayerStack) {
					layer->OnUpdate(timestep);
				}
			}

			// OnImGuiRender
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		PE_PROFILE_FUNCTION();
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PE_PROFILE_FUNCTION();
		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}