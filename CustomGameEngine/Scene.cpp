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

		SCR_WIDTH = this->sceneManager->GetWindowWidth();
		SCR_HEIGHT = this->sceneManager->GetWindowHeight();

		entityManager = new EntityManager();
		systemManager = new SystemManager();
		renderManager = RenderManager::GetInstance(1024 * 2, 1024 * 2, SCR_WIDTH, SCR_HEIGHT);
		renderManager->GetRenderParams()->SetRenderOptions(RENDER_UI | RENDER_SSAO | RENDER_SHADOWS | RENDER_BLOOM | RENDER_TONEMAPPING | RENDER_PARTICLES);
		constraintManager = new ConstraintManager();
		camera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 5.0f));
		dt = 0;

		renderManager->SetSkyboxTexture(ResourceManager::GetInstance()->LoadCubemap("Textures/Cubemaps/Space"));
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_SKYBOX);
		renderManager->GetRenderParams()->DisableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);

		renderManager->GetRenderParams()->SetBloomThreshold(15.0f);
		renderManager->GetRenderParams()->SetExposure(1.0f);
		renderManager->GetRenderParams()->SetBloomPasses(10);

		renderManager->GetRenderParams()->SetSSAOSamples(64);
		renderManager->GetRenderParams()->SetSSAORadius(0.5f);
		renderManager->GetRenderParams()->SetSSAOBias(0.025f);

		this->collisionManager = new CollisionManager();
	}

	Scene::~Scene()
	{
		delete camera;
		delete systemManager;
		delete constraintManager;
		delete entityManager;
	}

	void Scene::Update()
	{
		glm::vec3 position = camera->Position;
		glm::vec3 forward = camera->Front;
		AudioManager::GetInstance()->GetSoundEngine()->setListenerPosition(irrklang::vec3df(position.x, position.y, position.z), irrklang::vec3df(forward.x, forward.y, forward.z));
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