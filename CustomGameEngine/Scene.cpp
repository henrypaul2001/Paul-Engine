#include "Scene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "SystemManager.h"
namespace Engine
{
	float Scene::dt;

	Scene::Scene(SceneManager* sceneManager) {
		this->sceneManager = sceneManager;
		this->sceneManager->renderer = std::bind(&Scene::Render, this);
		this->sceneManager->updater = std::bind(&Scene::Update, this);

		entityManager = new EntityManager();
		systemManager = new SystemManager();
		renderManager = RenderManager::GetInstance(1024 * 7, 1024 * 7);

		SCR_WIDTH = this->sceneManager->GetWindowWidth();
		SCR_HEIGHT = this->sceneManager->GetWindowHeight();
		camera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 5.0f));
		dt = 0;
	}

	Scene::~Scene()
	{
		delete camera;
	}

	void Scene::Update()
	{

	}

	void Scene::Render()
	{
		glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, camera->NearClip, camera->FarClip);

		glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->GetViewMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(camera->Position));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	InputManager* Scene::GetInputManager()
	{
		return inputManager;
	}

	SystemManager* Scene::GetSystemManager()
	{
		return systemManager;
	}
}