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

		// Capture mouse
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// glad: load OpenGL function pointers
		// -----------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "FAIL::SCENEMANAGER::ONLOAD::Failed to initialize GLAD" << std::endl;
			glfwTerminate();
		}

		// Set up GL Debug output

		// Load GUI

		std::cout << "SUCCESS::SCENEMANAGER::ONLOAD::OpenGL initialised" << std::endl;
	}

	void SceneManager::OnUpdateFrame()
	{
		updater();
	}

	void SceneManager::OnRenderFrame()
	{
		renderer();

		// GL flush
		// Swap buffers
	}

	void SceneManager::Run()
	{
		// Temporary "game loop" standing in for future OpenGL game loop
		std::cout << "Starting new game" << std::endl;
		StartNewGame();
		
		float lastFrame = 0.0f;
		float currentFrame;
		while (!glfwWindowShouldClose(window))
		{
			currentFrame = static_cast<float>(glfwGetTime());
			Scene::dt = currentFrame - lastFrame;
			lastFrame = currentFrame;
			//Scene::dt = std::chrono::duration_cast<std::chrono::milliseconds>(timeStep).count() / 1000.0;

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