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
		SCR_WIDTH = *this->sceneManager->getWindowWidth();
		SCR_HEIGHT = *this->sceneManager->getWindowHeight();
		camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
		dt = 0;
	}

	Scene::~Scene()
	{

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