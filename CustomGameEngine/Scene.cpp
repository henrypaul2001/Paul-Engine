#include "Scene.h"
#include "SceneManager.h"
#include "InputManager.h"
namespace Engine
{
	float Scene::dt;

	Scene::Scene(SceneManager* sceneManager, const std::string& name) : rebuildBVHOnUpdate(false), SCR_WIDTH(sceneManager->GetWindowWidth()), SCR_HEIGHT(sceneManager->GetWindowHeight()), camera(new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 5.0f))), collisionManager(new CollisionManager()), constraintManager(new ConstraintManager()), systemManager(&ecs),
		collisionResolver(collisionManager),
		constraintSolver(constraintManager),
		audioSystem(&ecs),
		physicsSystem(&ecs),
		pathfindingSystem(&ecs),
		particleUpdater(&ecs),
		stateUpdater(&ecs),
		animSystem(&ecs),
		meshListSystem(&ecs),
		animAABBSystem(&ecs),

		aabbSystem(&ecs, collisionManager),
		boxSystem(&ecs, collisionManager),
		boxAABBSystem(&ecs, collisionManager),
		sphereSystem(&ecs, collisionManager),
		sphereAABBSystem(&ecs, collisionManager),
		sphereBoxSystem(&ecs, collisionManager),

		lightingSystem(&ecs, &lightManager, camera),

		uiInteract(&ecs, &inputManager) 
	{
		SCOPE_TIMER("Scene::Scene()");
		this->resources = ResourceManager::GetInstance();
		this->sceneManager = sceneManager;
		this->sceneManager->renderer = std::bind(&Scene::Render, this);
		this->sceneManager->updater = std::bind(&Scene::Update, this);

		renderManager = RenderManager::GetInstance();
		dt = 0;

		this->name = name;

		lightingSystem.SetActiveCamera(camera);
	}

	void Scene::OnSceneCreated()
	{
		systemManager.ActionPreUpdateSystems();
		collisionManager->ConstructBVHTree();
	}

	void Scene::Update()
	{
		SCOPE_TIMER("Scene::Update()");
		glm::vec3 position = camera->GetPosition();
		glm::vec3 forward = camera->GetFront();
		AudioManager::GetInstance()->GetSoundEngine()->setListenerPosition(irrklang::vec3df(position.x, position.y, position.z), irrklang::vec3df(forward.x, forward.y, forward.z));

		if (rebuildBVHOnUpdate) { collisionManager->ConstructBVHTree(); }
		frustumCulling.Run(camera, collisionManager);

		collisionResolver.Run(ecs);
		constraintSolver.Run(ecs);
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

		renderManager->GetRenderPipeline()->Run(&ecs, &lightManager, collisionManager, camera);
	}

	InputManager* Scene::GetInputManager() const
	{
		return inputManager;
	}

	SceneManager* Scene::GetSceneManager() const
	{
		return sceneManager;
	}
}