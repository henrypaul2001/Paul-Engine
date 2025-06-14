#include "pepch.h"
#include "OpenGLContext.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace PaulEngine {
	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
		// ignore warning codes or insignificant errors
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		PE_CORE_ERROR("OpenGL Error:");
		PE_CORE_ERROR("Debug message ({0}): {1}", id, message);

		switch (source) {
			case GL_DEBUG_SOURCE_API:
				PE_CORE_ERROR("Source: API"); break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				PE_CORE_ERROR("Source: Window System"); break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				PE_CORE_ERROR("Source: Shader Compiler"); break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				PE_CORE_ERROR("Source: Third Party"); break;
			case GL_DEBUG_SOURCE_APPLICATION:
				PE_CORE_ERROR("Source: Application"); break;
			case GL_DEBUG_SOURCE_OTHER:
				PE_CORE_ERROR("Source: Other"); break;
		}

		switch (type) {
			case GL_DEBUG_TYPE_ERROR:
				PE_CORE_ERROR("Type: Error"); break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				PE_CORE_ERROR("Type: Deprecated Behaviour"); break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				PE_CORE_ERROR("Type: Undefined Behaviour"); break;
			case GL_DEBUG_TYPE_PORTABILITY:
				PE_CORE_ERROR("Type: Portability"); break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				PE_CORE_ERROR("Type: Performance"); break;
			case GL_DEBUG_TYPE_MARKER:
				PE_CORE_ERROR("Type: Marker"); break;
			case GL_DEBUG_TYPE_PUSH_GROUP:
				PE_CORE_ERROR("Type: Push Group"); break;
			case GL_DEBUG_TYPE_POP_GROUP:
				PE_CORE_ERROR("Type: Pop Group"); break;
			case GL_DEBUG_TYPE_OTHER:
				PE_CORE_ERROR("Type: Other"); break;
		}

		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:
				PE_CORE_ERROR("Type: High"); break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				PE_CORE_ERROR("Type: Medium"); break;
			case GL_DEBUG_SEVERITY_LOW:
				PE_CORE_ERROR("Type: Low"); break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				PE_CORE_ERROR("Type: Notification"); break;
		}
	}

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

		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			PE_CORE_INFO("    Debug output: true");
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
		else { PE_CORE_INFO("    Debug output: false"); }
	}

	void OpenGLContext::SwapBuffers()
	{
		PE_PROFILE_FUNCTION();
		glfwSwapBuffers(m_Window);
	}
}