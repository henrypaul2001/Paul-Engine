#include "Scene.h"
namespace Engine
{
	Scene::Scene(SceneManager* sceneManager) {
		this->sceneManager = sceneManager;
		this->sceneManager->renderer = std::bind(&Scene::Render, this);
		this->sceneManager->updater = std::bind(&Scene::Update, this);
		dt = 0;
	}

	Scene::~Scene()
	{
		Close();
	}
}