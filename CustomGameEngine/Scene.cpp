#include "Scene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "SystemManager.h"
namespace Engine
{
	float Scene::dt;

	Scene::Scene(SceneManager* sceneManager, const std::string& name) : camera(new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 5.0f))), collisionManager(new CollisionManager()) {
		SCOPE_TIMER("Scene::Scene()");
		this->resources = ResourceManager::GetInstance();
		this->sceneManager = sceneManager;
		this->sceneManager->renderer = std::bind(&Scene::Render, this);
		this->sceneManager->updater = std::bind(&Scene::Update, this);

		SCR_WIDTH = this->sceneManager->GetWindowWidth();
		SCR_HEIGHT = this->sceneManager->GetWindowHeight();

		entityManager = new EntityManager();
		systemManager = new SystemManager();
		renderManager = RenderManager::GetInstance();
		constraintManager = new ConstraintManager();
		dt = 0;

		this->name = name;
	}

	Scene::~Scene()
	{
		ResourceManager::GetInstance()->ClearTempResources();
		renderManager->GetBakedData().ClearBakedData();
		delete camera;
		delete systemManager;
		delete constraintManager;
		delete entityManager;
	}

	void Scene::OnSceneCreated()
	{
		//collisionManager->ConstructBVHTree();
	}

	void Scene::Update()
	{
		SCOPE_TIMER("Scene::Update()");
		glm::vec3 position = camera->GetPosition();
		glm::vec3 forward = camera->GetFront();
		AudioManager::GetInstance()->GetSoundEngine()->setListenerPosition(irrklang::vec3df(position.x, position.y, position.z), irrklang::vec3df(forward.x, forward.y, forward.z));

		collisionManager->ConstructBVHTree();
		frustumCulling.Run(camera, collisionManager);
	}

	void Scene::Render()
	{
		SCOPE_TIMER("Scene::Render()");
		glm::mat4 projection = glm::perspective(glm::radians(camera->GetZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, camera->GetNearClip(), camera->GetFarClip());

		glBindBuffer(GL_UNIFORM_BUFFER, ResourceManager::GetInstance()->CommonUniforms());
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->GetViewMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::vec3), glm::value_ptr(camera->GetPosition()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	InputManager* Scene::GetInputManager() const
	{
		return inputManager;
	}

	SystemManager* Scene::GetSystemManager() const
	{
		return systemManager;
	}

	SceneManager* Scene::GetSceneManager() const
	{
		return sceneManager;
	}
}