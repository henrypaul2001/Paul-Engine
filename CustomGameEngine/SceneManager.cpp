#include "SceneManager.h"
#include "InputManager.h"
namespace Engine
{
	Shader* SceneManager::defaultLit = nullptr;

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
		delete defaultLit;
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

		// Configure default shaders
		// -------------------------

		defaultLit = ResourceManager::GetInstance()->LoadShader("Shaders/defaultLitNew.vert", "Shaders/defaultLitNew.frag");
		defaultLit->Use();
		defaultLit->setInt("dirLight.ShadowMap", 0);
		for (int i = 0; i <= 8; i++) {
			defaultLit->setInt((std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowMap"))), i + 1);
		}

		defaultLit->setInt("material.TEXTURE_DIFFUSE1", 10);
		defaultLit->setInt("material.TEXTURE_SPECULAR1", 11);
		defaultLit->setInt("material.TEXTURE_NORMAL1", 12);
		defaultLit->setInt("material.TEXTURE_DISPLACE1", 13);

		// Uniform blocks
		unsigned int defaultLitBlockLocation = glGetUniformBlockIndex(defaultLit->GetID(), "Common");
		// unsigned int defaultLitPBRBlockLocation = glGetUniformBlockIndex(defaultLit_pbr.GetID(), "Matrices");
		glUniformBlockBinding(defaultLit->GetID(), defaultLitBlockLocation, 0);
		// same again for pbr

		glGenBuffers(1, &uboMatrices);
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4) + sizeof(glm::vec3));

		std::cout << "SUCCESS::SCENEMANAGER::ONLOAD::OpenGL initialised" << std::endl;
	}

	void SceneManager::OnUpdateFrame()
	{
		// Call scene update delegate
		updater();
	}

	void SceneManager::OnRenderFrame(Camera* camera)
	{
		// Configure default shaders
		// -------------------------
		defaultLit->Use();
		defaultLit->setBool("gamma", false);

		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 600.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->GetViewMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(camera->Position));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Call scene render delegate
		renderer();

		// GL flush
		// Swap buffers
	}

	void SceneManager::Run()
	{
		std::cout << "Starting new game" << std::endl;
		StartNewGame();

		// Set up camera pointers
		Camera* camera = &scene->camera; // no need for this at all. Just have it all done in the scene class
		scene->GetInputManager()->SetCameraPointer(camera);
		//static_cast<SystemRender*>(scene->GetSystemManager()->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS))->SetCameraPointer(camera);

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
			OnRenderFrame(camera);

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