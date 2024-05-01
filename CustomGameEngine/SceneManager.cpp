#include "SceneManager.h"
#include "InputManager.h"
namespace Engine
{
	SceneManager::SceneManager(int width, int height, int windowXPos, int windowYPos) 
	{
		this->SCR_WIDTH = width;
		this->SCR_HEIGHT = height;
		this->windowXPos = windowXPos;
		this->windowYPos = windowYPos;
		OnLoad();
	}

	SceneManager::~SceneManager()
	{
	}

	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
		// ignore warning codes or insignificant errors
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;

		switch (source) {
		case GL_DEBUG_SOURCE_API:
			std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:
			std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:
			std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:
			std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:
			std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:
			std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:
			std::cout << "Type: Other"; break;
		}	std::cout << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:
			std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
	}

	void SceneManager::OnLoad()
	{
		// glfw: initialize and configure
		// ------------------------------
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		// glfw: create window
		// -------------------
		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Paul Engine", NULL, NULL);
		glfwMakeContextCurrent(window);
		if (window == NULL) {
			std::cout << "FAIL::SCENEMANAGER::ONLOAD::Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		//glfwSetCursorPosCallback(window, mouse_callback);
		//glfwSetScrollCallback(window, scroll_callback);
		//glfwSwapInterval(0); // disables v sync

		// Capture mouse
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// glad: load OpenGL function pointers
		// -----------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "FAIL::SCENEMANAGER::ONLOAD::Failed to initialize GLAD" << std::endl;
			glfwTerminate();
		}

		// Set up GL Debug output
		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageCallback(glDebugOutput, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			//glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_MEDIUM, -1, "error message here"); // example of custom error message
		}

		// Setup freetype
		if (FT_Init_FreeType(&freetypeLib)) {
			std::cout << "FAIL::SCENEMANAGER::ONLOAD::Failed to initialize FreeType Library" << std::endl;
		}

		// Set up Resource Manager
		resources = ResourceManager::GetInstance();

		std::cout << "SUCCESS::SCENEMANAGER::ONLOAD::OpenGL initialised" << std::endl;
	}

	void SceneManager::OnUpdateFrame()
	{
		// Call scene update delegate
		updater();
	}

	void SceneManager::OnRenderFrame()
	{
		// Call scene render delegate
		renderer();
	}

	void SceneManager::Run()
	{
		std::cout << "Starting new game" << std::endl;
		StartNewGame();

		float lastFrame = static_cast<float>(glfwGetTime());
		float currentFrame;
		Scene::dt = 0.0f;
		while (!glfwWindowShouldClose(window))
		{
			currentFrame = static_cast<float>(glfwGetTime()) + 0.0001f;
			Scene::dt = currentFrame - lastFrame;
			lastFrame = currentFrame;
			//Scene::dt = std::chrono::duration_cast<std::chrono::milliseconds>(timeStep).count() / 1000.0;
			//Scene::dt = 0.0066f;
			// Process inputs
			scene->GetInputManager()->ProcessInputs();

			// Update scene
			OnUpdateFrame();

			// Render scene
			OnRenderFrame();

			// Swap buffers
			glfwSwapBuffers(window);

			// Poll events
			glfwPollEvents();

			// Prepare for next frame
			// 
			//std::cout << "FPS: " << (1.0f / Scene::dt) << std::endl;

			/*
			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> frameDuration = endTime - currentTime;
			std::chrono::duration<double> remainingTime = std::chrono::duration<double>(targetFrameTime) - frameDuration;

			if (remainingTime.count() > 0) {
				std::this_thread::sleep_for(remainingTime);
			}
			*/
			/*
			auto nextFrameTime = currentTime + std::chrono::nanoseconds(16666667);
			// Sleep until nextFrameTime
			auto sleepDuration = nextFrameTime - std::chrono::high_resolution_clock::now();
			if (sleepDuration > std::chrono::nanoseconds(0)) {
				std::this_thread::sleep_for(sleepDuration);
			}
			*/
		}
	}
}