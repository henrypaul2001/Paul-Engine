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
		PE_PROFILE_FUNCTION();
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		PE_CORE_ASSERT(status, "Failed to initialise Glad!");

		const char* vendorInfo = (const char*)glGetString(GL_VENDOR);
		const char* rendererInfo = (const char*)glGetString(GL_RENDERER);
		const char* versionInfo = (const char*)glGetString(GL_VERSION);
		PE_CORE_INFO("OpenGL Info:");
		PE_CORE_INFO("    Vendor: {0}", vendorInfo);
		PE_CORE_INFO("    Renderer: {0}", rendererInfo);
		PE_CORE_INFO("    Version: {0}", versionInfo);
	}

	void OpenGLContext::SwapBuffers()
	{
		PE_PROFILE_FUNCTION();
		glfwSwapBuffers(m_Window);
	}
}