#include "ParticleScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
#include "SystemParticleUpdater.h"
#include "SystemParticleRenderer.h"
#include "SystemCollisionSphereAABB.h"
#include "SystemCollisionSphereBox.h"
#include "SystemPhysics.h"
namespace Engine {
	ParticleScene::ParticleScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		//renderManager->GetRenderParams()->DisableRenderOptions(RENDER_SKYBOX);
		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	ParticleScene::~ParticleScene()
	{

	}

	void ParticleScene::ChangePostProcessEffect()
	{
		SystemRender* renderSystem = dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS));
		unsigned int currentEffect = renderSystem->GetPostProcess();
		unsigned int nextEffect;
		if (currentEffect == 8u) {
			nextEffect = 0u;
		}
		else {
			nextEffect = currentEffect + 1;
		}

		dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS))->SetPostProcess((PostProcessingEffect)nextEffect);
	}

	void ParticleScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);

		float time = (float)glfwGetTime();

		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) {
			targetFPSPercentage = 1.0f;
		}

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		Entity* ball = entityManager->FindEntity("Smoke Particles");
		ComponentPhysics* ballPhysics = ball->GetPhysicsComponent();
		float forceMagnitude = 0.3f;

		// Lava ball controls
		if (inputManager->IsKeyDown(GLFW_KEY_KP_6)) {
			// Move right
			ballPhysics->AddForce(glm::vec3(forceMagnitude, 0.0f, 0.0f));
			ballPhysics->AddTorque(glm::vec3(0.0f, 0.0f, -forceMagnitude * 200.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_4)) {
			// Move left
			ballPhysics->AddForce(glm::vec3(-forceMagnitude, 0.0f, 0.0f));
			ballPhysics->AddTorque(glm::vec3(0.0f, 0.0f, forceMagnitude * 200.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_8)) {
			// Move up
			ballPhysics->AddForce(glm::vec3(0.0f, 0.0f, -forceMagnitude));
			ballPhysics->AddTorque(glm::vec3(-forceMagnitude * 200.0f, 0.0f, 0.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_5)) {
			// Move down
			ballPhysics->AddForce(glm::vec3(0.0f, 0.0f, forceMagnitude));
			ballPhysics->AddTorque(glm::vec3(forceMagnitude * 200.0f, 0.0f, 0.0f));
		}
	}

	void ParticleScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void ParticleScene::Close()
	{

	}

	void ParticleScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void ParticleScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
	}

	void ParticleScene::keyDown(int key)
	{
		Entity* ball = entityManager->FindEntity("Smoke Particles");
		ComponentPhysics* ballPhysics = ball->GetPhysicsComponent();
		float forceMagnitude = 0.3f;

		if (key == GLFW_KEY_KP_2) {
			// Jump
			ballPhysics->AddForce(glm::vec3(0.0f, 150.0f, 0.0f));
		}
	}

	void ParticleScene::CreateEntities()
	{
		Material* windowMaterial = new Material();
		windowMaterial->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		windowMaterial->specular = glm::vec3(0.8f, 0.0f, 0.0f);
		windowMaterial->shininess = 60.0f;
		windowMaterial->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/window/window.png", TEXTURE_DIFFUSE, true));
		windowMaterial->opacityMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/window/window_opacity.png", TEXTURE_OPACITY, false));
		windowMaterial->isTransparent = true;

		Material* lava = new Material();
		lava->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/Lava/diffuse.png", TEXTURE_DIFFUSE, true));
		lava->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/Lava/specular.png", TEXTURE_SPECULAR, false));
		//lava->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/Lava/normal.png", TEXTURE_NORMAL, false));
		lava->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/Lava/displace.png", TEXTURE_DISPLACE, false));
		lava->height_scale = -0.001f;
		lava->shininess = 0.2f;

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		//directional->Colour = glm::vec3(1.0f, 1.0f, 1.5f) * 10.0f;
		directional->Colour = glm::vec3(0.7f, 0.65f, 0.85f);
		directional->Specular = glm::vec3(0.7f, 0.65f, 0.85f);
		directional->ShadowProjectionSize = 10.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		floor->GetTransformComponent()->SetScale(glm::vec3(10.0f, 0.5f, 10.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		floor->GetAABBCollisionComponent()->IsMovedByCollisions(false);
		entityManager->AddEntity(floor);

		RandomParameters wallParams;
		wallParams.randomPositionXRange = glm::vec2(-0.05f, 0.05f);
		wallParams.randomPositionYRange = glm::vec2(0.0f);
		wallParams.randomPositionZRange = glm::vec2(-0.05f, 10.05f);
		wallParams.randomVelocityXRange = glm::vec2(-1.25f, 1.25f);
		wallParams.randomVelocityYRange = glm::vec2(5.5f, 7.5f);
		wallParams.randomVelocityZRange = glm::vec2(-1.25, 1.25f);

		RandomParameters smokeParams;
		smokeParams.randomPositionXRange = glm::vec2(-0.15f, 0.15f);
		smokeParams.randomPositionYRange = glm::vec2(0.65f, 1.25f);
		smokeParams.randomPositionZRange = smokeParams.randomPositionXRange;
		smokeParams.randomVelocityXRange = glm::vec2(-0.0015f, -0.0005f);
		smokeParams.randomVelocityYRange = glm::vec2(0.70f, 0.75f);
		smokeParams.randomVelocityZRange = glm::vec2(0.0f);
		smokeParams.randomAccelerationXRange = glm::vec2(-0.000030f, -0.000029f);

		RandomParameters energyParams;
		energyParams.randomPositionXRange = glm::vec2(-0.1f, 0.1f);
		energyParams.randomPositionYRange = glm::vec2(0.0f);
		energyParams.randomPositionZRange = glm::vec2 (-0.1f, 0.1f);
		energyParams.randomAccelerationXRange = glm::vec2(-0.015f, 0.015f);
		energyParams.randomAccelerationYRange = energyParams.randomAccelerationXRange;
		energyParams.randomAccelerationZRange = energyParams.randomAccelerationXRange;

		Entity* particles = new Entity("Particles");
		particles->AddComponent(new ComponentTransform(0.0f, 0.8f, 0.0f));
		particles->AddComponent(new ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false), 3000, glm::vec3(0.0f), 150.0f, 0.5f, 0.5f, glm::vec3(1.0f), 0.5f));
		particles->GetParticleGenerator()->SetRandomParameters(wallParams);
		entityManager->AddEntity(particles);

		Entity* particles2 = new Entity("Particles 2");
		particles2->AddComponent(new ComponentTransform(5.0f, 0.8f, 0.0f));
		particles2->AddComponent(new ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false), 15000, glm::vec3(0.0f), 3000.0f, 0.5f, 0.5f, glm::vec3(1.0f), 0.5f));
		particles2->GetParticleGenerator()->SetRandomParameters(wallParams);
		entityManager->AddEntity(particles2);

		Entity* particles3 = new Entity("Particles 3");
		particles3->AddComponent(new ComponentTransform(-5.0f, 0.8f, 0.0f));
		particles3->AddComponent(new ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false), 3000, glm::vec3(0.0f), 10.0f, 0.5f, 0.5f, glm::vec3(1.0f), 0.5f));
		particles3->GetParticleGenerator()->SetRandomParameters(wallParams);
		entityManager->AddEntity(particles3);

		Entity* energyParticles = new Entity("Energy Particles");
		energyParticles->AddComponent(new ComponentTransform(0.0f, 2.0f, -4.5f));
		energyParticles->AddComponent(new ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/energy.png", TEXTURE_DIFFUSE, false), 500, glm::vec3(0.0f), 50.0f, 0.5f, 0.5f, glm::vec3(0.25f), 0.5f, 0.45f, 5.0f));
		energyParticles->GetParticleGenerator()->SetRandomParameters(energyParams);
		entityManager->AddEntity(energyParticles);

		ComponentLight* lavaRockLight = new ComponentLight(POINT);
		lavaRockLight->Colour = glm::vec3(161.0f, 36.0f, 36.0f) / 255.0f;
		lavaRockLight->Specular = lavaRockLight->Colour;
		lavaRockLight->Ambient = glm::vec3(lavaRockLight->Colour * 0.2f);
		lavaRockLight->CastShadows = false;
		lavaRockLight->Constant = 1.0f;
		lavaRockLight->Linear = 0.35f;
		lavaRockLight->Quadratic = 0.44f;

		Entity* smokeParticles = new Entity("Smoke Particles");
		smokeParticles->AddComponent(new ComponentTransform(-7.0f, 3.0f, -6.5f));
		smokeParticles->GetTransformComponent()->SetScale(0.5f);
		smokeParticles->AddComponent(new ComponentGeometry(MODEL_SPHERE, false));
		smokeParticles->GetGeometryComponent()->ApplyMaterialToModel(lava);
		smokeParticles->AddComponent(new ComponentCollisionSphere(1.0f));
		smokeParticles->AddComponent(new ComponentPhysics(20.0f, 1.05f, 1.0f, 0.4f, true, false));
		smokeParticles->AddComponent(new ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/smoke2.png", TEXTURE_DIFFUSE, false), 3000, glm::vec3(0.0f), 10.0f, 25.0f, 0.5f, glm::vec3(0.5f), 0.5f, 0.0f, 0.0f, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		smokeParticles->GetParticleGenerator()->SetRandomParameters(smokeParams);
		smokeParticles->AddComponent(lavaRockLight);
		entityManager->AddEntity(smokeParticles);

		Entity* orbParticles = new Entity("Orb Particles");
		orbParticles->AddComponent(new ComponentTransform(7.0f, 2.0f, -5.0f));
		orbParticles->AddComponent(new ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/orb.png", TEXTURE_DIFFUSE, false), 3, glm::vec3(0.0f), 1.0f, 1.0f, 0.5f, glm::vec3(2.5f)));
		orbParticles->GetParticleGenerator()->SetRandomParameters(energyParams);
		entityManager->AddEntity(orbParticles);

		Entity* window = new Entity("Window");
		window->AddComponent(new ComponentTransform(10.0f, 3.0f, 2.5f));
		window->AddComponent(new ComponentGeometry(MODEL_PLANE));
		window->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(windowMaterial);
		entityManager->AddEntity(window);

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(30.0f, 80.0f), glm::vec2(0.25f, 0.25f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(30.0f, 30.0f), glm::vec2(0.20f, 0.20f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		entityManager->AddEntity(canvas);
	}

	void ParticleScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(new SystemParticleUpdater(), UPDATE_SYSTEMS);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemParticleRenderer(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereBox(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddCollisionResponseSystem(new CollisionResolver(collisionManager));
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
	}
}