#include "SponzaScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
namespace Engine {
	SponzaScene::SponzaScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		entityManager = new EntityManager();
		systemManager = new SystemManager();
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		renderManager = RenderManager::GetInstance(1024 * 2, 1024 * 2, SCR_WIDTH, SCR_HEIGHT);

		SetupScene();
	}

	SponzaScene::~SponzaScene()
	{
		delete entityManager;
		delete systemManager;
		delete inputManager;
	}

	void SponzaScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Set-up framebuffers

		// Compile shaders

		// Configure shaders

		// Prepare scene

		CreateSystems();
		CreateEntities();
	}

	void SponzaScene::keyUp(int key)
	{

	}

	void SponzaScene::keyDown(int key)
	{

	}

	void SponzaScene::CreateEntities()
	{
		PBRMaterial* gold = new PBRMaterial();
		gold->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO));

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		dirLight->AddComponent(new ComponentLight(DIRECTIONAL));
		dynamic_cast<ComponentLight*>(dirLight->GetComponent(COMPONENT_LIGHT))->Direction = glm::vec3(0.35f, -1.0f, 0.0f);
		dynamic_cast<ComponentLight*>(dirLight->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(50.0f, 50.0f, 50.0f);
		entityManager->AddEntity(dirLight);

		Entity* sponza = new Entity("Sponza");
		sponza->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		sponza->AddComponent(new ComponentGeometry("Models/sponza/sponza.obj", true));
		dynamic_cast<ComponentGeometry*>(sponza->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		entityManager->AddEntity(sponza);
		
		Entity* light = new Entity("Light");
		light->AddComponent(new ComponentTransform(0.0f, 15.0f, 0.0f));
		light->AddComponent(new ComponentLight(POINT));
		light->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentLight*>(light->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(100.0f, 100.0f, 100.0f);
		dynamic_cast<ComponentGeometry*>(light->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(light->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(light->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(light->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		entityManager->AddEntity(light);

		Entity* light2 = new Entity("Light 2");
		light2->AddComponent(new ComponentTransform(0.0f, 10.0f, 0.0f));
		light2->AddComponent(new ComponentLight(POINT));
		light2->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentLight*>(light2->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(100.0f, 100.0f, 100.0f);
		dynamic_cast<ComponentGeometry*>(light2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(light2->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(light2->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(light2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		entityManager->AddEntity(light2);

		Entity* light3 = new Entity("Light 3");
		light3->AddComponent(new ComponentTransform(0.0f, 2.0f, 0.0f));
		light3->AddComponent(new ComponentLight(POINT));
		dynamic_cast<ComponentLight*>(light3->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(100.0f, 100.0f, 100.0f);
		light3->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(light3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(light3->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(light3->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(light3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		entityManager->AddEntity(light3);

		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(0.0f, 6.5f, 0.0f));
		spotLight->AddComponent(new ComponentLight(SPOT));
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Direction = glm::vec3(0.0f, -0.1f, 1.0f);
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Cutoff = glm::radians(12.5f);
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->OuterCutoff = glm::radians(15.0f);
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Linear = 0.045f;
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Quadratic = 0.0075f;
		entityManager->AddEntity(spotLight);
	}

	void SponzaScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
	}

	void SponzaScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);
		float time = (float)glfwGetTime();

		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::rotate(model, glm::radians(10.0f) * sin(time), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::vec3 newDirection = model * glm::vec4(dynamic_cast<ComponentLight*>(entityManager->FindEntity("Spot Light")->GetComponent(COMPONENT_LIGHT))->Direction, 1.0f);
		//dynamic_cast<ComponentLight*>(entityManager->FindEntity("Spot Light")->GetComponent(COMPONENT_LIGHT))->Direction = newDirection;
	}

	void SponzaScene::Render()
	{
		Scene::Render();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// Render scene
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void SponzaScene::Close()
	{

	}
}