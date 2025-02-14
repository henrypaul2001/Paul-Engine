#include "pepch.h"
#include "OpenGLContext.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace PaulEngine {
	OpenGLContext::OpenGLContext(GLFWwindow* window) : m_Window(window)
	{
		PE_CORE_ASSERT(window, "Window is null!");
	}

	OpenGLContext::~OpenGLContext()
	{

	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		PE_CORE_ASSERT(status, "Failed to initialise Glad!");
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}
}