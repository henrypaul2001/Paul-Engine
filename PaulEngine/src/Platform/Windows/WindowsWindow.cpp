#include "pepch.h"
#include "WindowsWindow.h"

#include "PaulEngine/Events/ApplicationEvent.h"
#include "PaulEngine/Events/MouseEvent.h"
#include "PaulEngine/Events/KeyEvent.h"

namespace PaulEngine {
	static bool s_GLFWInitialised = false;

	static void GLFWErrorCallback(int error, const char* message) {
		PE_CORE_ERROR("GLFW Error: ({0}) {1}", error, message);
	}

	Window* Window::Create(const WindowProperties& properties) { return new WindowsWindow(properties); }

	WindowsWindow::WindowsWindow(const WindowProperties& properties) { Init(properties); }

	WindowsWindow::~WindowsWindow() { Shutdown(); }

	void WindowsWindow::Init(const WindowProperties& properties)
	{
		m_Data.Title = properties.Title;
		m_Data.Width = properties.Width;
		m_Data.Height = properties.Height;

		PE_CORE_INFO("Creating Windows window: {0} ({1}, {2})", properties.Title, properties.Width, properties.Height);

		if (!s_GLFWInitialised) {
			int success = glfwInit();
			PE_CORE_ASSERT(success, "Failed to initialise GLFW!");

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialised = true;
		}

		m_Window = glfwCreateWindow((int)properties.Width, (int)properties.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// GLFW event callbacks
		// --------------------
		// Window resize
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.eventCallback(event);
		});

		// Window close
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.eventCallback(event);
		});

		// Keyboard
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
				case GLFW_PRESS:
				{
					KeyPressedEvent event = KeyPressedEvent(key, 0);
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event = KeyReleasedEvent(key);
					data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event = KeyPressedEvent(key, 1);
					data.eventCallback(event);
					break;
				}
			}
		});

		// Mouse button
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action) {
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event = MouseButtonPressedEvent(button);
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event = MouseButtonReleasedEvent(button);
					data.eventCallback(event);
					break;
				}
			}
		});

		// Mouse scroll
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrolledEvent event = MouseScrolledEvent((float)xoffset, (float)yoffset);
			data.eventCallback(event);
		});

		// Mouse move
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event = MouseMovedEvent((float)xpos, (float)ypos);
			data.eventCallback(event);
		});
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled);
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}