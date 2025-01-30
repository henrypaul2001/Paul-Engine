#include "ParticleScene.h"
#include "GameInputManager.h"
#include "UIText.h"
namespace Engine {
	ParticleScene::ParticleScene(SceneManager* sceneManager) : Scene(sceneManager, "ParticleScene")
	{
		rebuildBVHOnUpdate = true;
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		//renderManager->GetRenderParams()->DisableRenderOptions(RENDER_SKYBOX);
		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	ParticleScene::~ParticleScene() {}

	void ParticleScene::ChangePostProcessEffect()
	{
		SystemRender& renderSystem = renderManager->GetRenderPipeline()->GetRenderSystem();
		unsigned int currentEffect = renderSystem.GetPostProcess();
		unsigned int nextEffect;
		if (currentEffect == 8u) {
			nextEffect = 0u;
		}
		else {
			nextEffect = currentEffect + 1;
		}

		renderSystem.SetPostProcess((PostProcessingEffect)nextEffect);
	}

	void ParticleScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		float time = (float)glfwGetTime();
		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) { targetFPSPercentage = 1.0f; }

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		Entity* canvasEntity = ecs.Find("Canvas");
		ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(canvasEntity->ID());
		dynamic_cast<UIText*>(canvas->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(canvas->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		BVHTree* geometryBVH = collisionManager->GetBVHTree();

		const unsigned int meshCount = frustumCulling.GetTotalMeshes();
		const unsigned int visibleMeshes = frustumCulling.GetVisibleMeshes();
		const unsigned int nodeCount = geometryBVH->GetNodeCount();
		const unsigned int aabbTests = frustumCulling.GetTotalAABBTests();

		dynamic_cast<UIText*>(canvas->UIElements()[6])->SetText("Mesh count: " + std::to_string(meshCount));
		dynamic_cast<UIText*>(canvas->UIElements()[7])->SetText("     - Visible: " + std::to_string(visibleMeshes));
		dynamic_cast<UIText*>(canvas->UIElements()[8])->SetText("BVHN count: " + std::to_string(nodeCount));
		dynamic_cast<UIText*>(canvas->UIElements()[9])->SetText("AABB Tests: " + std::to_string(aabbTests));

		Entity* ball = ecs.Find("Smoke Particles");
		ComponentPhysics* ballPhysics = ecs.GetComponent<ComponentPhysics>(ball->ID());
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

		systemManager.ActionSystems();
	}

	void ParticleScene::Render()
	{
		Scene::Render();
	}

	void ParticleScene::Close() {}

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
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			Entity* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void ParticleScene::keyDown(int key)
	{
		Entity* ball = ecs.Find("Smoke Particles");
		ComponentPhysics* ballPhysics = ecs.GetComponent<ComponentPhysics>(ball->ID());
		float forceMagnitude = 0.3f;

		if (key == GLFW_KEY_KP_2) {
			// Jump
			ballPhysics->AddForce(glm::vec3(0.0f, 150.0f, 0.0f));
		}
	}

	void ParticleScene::CreateEntities()
	{
		Material* windowMaterial = new Material();
		windowMaterial->baseColour = glm::vec3(1.0f, 1.0f, 1.0f);
		windowMaterial->specular = glm::vec3(0.8f, 0.0f, 0.0f);
		windowMaterial->shininess = 60.0f;
		windowMaterial->baseColourMaps.push_back(resources->LoadTexture("Materials/window/window.png", TEXTURE_DIFFUSE, true));
		windowMaterial->PushOpacityMap(resources->LoadTexture("Materials/window/window_opacity.png", TEXTURE_OPACITY, false));
		windowMaterial->shadowCastAlphaDiscardThreshold = 0.5f;
		resources->AddMaterial("windowMaterial", windowMaterial);

		Material* lava = new Material();
		lava->baseColourMaps.push_back(resources->LoadTexture("Materials/Lava/diffuse.png", TEXTURE_DIFFUSE, true));
		lava->specularMaps.push_back(resources->LoadTexture("Materials/Lava/specular.png", TEXTURE_SPECULAR, false));
		//lava->normalMaps.push_back(resources->LoadTexture("Materials/Lava/normal.png", TEXTURE_NORMAL, false));
		lava->heightMaps.push_back(resources->LoadTexture("Materials/Lava/displace.png", TEXTURE_DISPLACE, false));
		lava->height_scale = -0.001f;
		lava->shininess = 0.2f;
		resources->AddMaterial("lava", lava);

		Entity* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		//directional.Colour = glm::vec3(1.0f, 1.0f, 1.5f) * 10.0f;
		directional.Colour = glm::vec3(0.7f, 0.65f, 0.85f);
		directional.Specular = glm::vec3(0.7f, 0.65f, 0.85f);
		directional.ShadowProjectionSize = 15.0f;
		directional.Far = 150.0f;
		ecs.AddComponent(dirLight->ID(), directional);

		Entity* floor = ecs.New("Floor");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetScale(glm::vec3(10.0f, 0.5f, 10.0f));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.AddComponent(floor->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(floor->ID())->IsMovedByCollisions(false);

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

		Entity* particles = ecs.New("Particles");
		transform = ecs.GetComponent<ComponentTransform>(particles->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.8f, 0.0f));
		ecs.AddComponent(particles->ID(), ComponentParticleGenerator(resources->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false), 3000, glm::vec3(0.0f), 150.0f, 0.5f, 0.5f, glm::vec3(1.0f), 0.5f));
		ecs.GetComponent<ComponentParticleGenerator>(particles->ID())->SetRandomParameters(wallParams);

		Entity* particles2 = ecs.New("Particles 2");
		transform = ecs.GetComponent<ComponentTransform>(particles2->ID());
		transform->SetPosition(glm::vec3(5.0f, 0.8f, 0.0f));
		ecs.AddComponent(particles2->ID(), ComponentParticleGenerator(resources->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false), 15000, glm::vec3(0.0f), 3000.0f, 0.5f, 0.5f, glm::vec3(1.0f), 0.5f));
		ecs.GetComponent<ComponentParticleGenerator>(particles2->ID())->SetRandomParameters(wallParams);

		Entity* particles3 = ecs.New("Particles 3");
		transform = ecs.GetComponent<ComponentTransform>(particles3->ID());
		transform->SetPosition(glm::vec3(-5.0f, 0.8f, 0.0f));
		ecs.AddComponent(particles3->ID(), ComponentParticleGenerator(resources->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false), 3000, glm::vec3(0.0f), 10.0f, 0.5f, 0.5f, glm::vec3(1.0f), 0.5f));
		ecs.GetComponent<ComponentParticleGenerator>(particles3->ID())->SetRandomParameters(wallParams);

		Entity* energyParticles = ecs.New("Energy Particles");
		transform = ecs.GetComponent<ComponentTransform>(energyParticles->ID());
		transform->SetPosition(glm::vec3(0.0f, 2.0f, -4.5f));
		ecs.AddComponent(energyParticles->ID(), ComponentParticleGenerator(resources->LoadTexture("Textures/Particles/energy.png", TEXTURE_DIFFUSE, false), 500, glm::vec3(0.0f), 50.0f, 0.5f, 0.5f, glm::vec3(0.25f), 0.5f, 0.45f, 5.0f));
		ecs.GetComponent<ComponentParticleGenerator>(energyParticles->ID())->SetRandomParameters(energyParams);

		ComponentLight lavaRockLight = ComponentLight(POINT);
		lavaRockLight.Colour = glm::vec3(161.0f, 36.0f, 36.0f) / 255.0f;
		lavaRockLight.Specular = lavaRockLight.Colour;
		lavaRockLight.Ambient = glm::vec3(lavaRockLight.Colour * 0.2f);
		lavaRockLight.CastShadows = false;
		lavaRockLight.Constant = 1.0f;
		lavaRockLight.Linear = 0.35f;
		lavaRockLight.Quadratic = 0.44f;

		Entity* smokeParticles = ecs.New("Smoke Particles");
		transform = ecs.GetComponent<ComponentTransform>(smokeParticles->ID());
		transform->SetPosition(glm::vec3(-7.0f, 3.0f, -6.5f));
		transform->SetScale(0.5f);
		ecs.AddComponent(smokeParticles->ID(), ComponentGeometry(MODEL_SPHERE, false));
		ecs.GetComponent<ComponentGeometry>(smokeParticles->ID())->ApplyMaterialToModel(lava);
		ecs.AddComponent(smokeParticles->ID(), ComponentCollisionSphere(1.0f));
		ecs.AddComponent(smokeParticles->ID(), ComponentPhysics(20.0f, 1.05f, 1.0f, 0.4f, true, false));
		ecs.AddComponent(smokeParticles->ID(), ComponentParticleGenerator(resources->LoadTexture("Textures/Particles/smoke2.png", TEXTURE_DIFFUSE, false), 3000, glm::vec3(0.0f), 10.0f, 25.0f, 0.5f, glm::vec3(0.5f), 0.5f, 0.0f, 0.0f, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		ecs.GetComponent<ComponentParticleGenerator>(smokeParticles->ID())->SetRandomParameters(smokeParams);
		ecs.AddComponent(smokeParticles->ID(), lavaRockLight);

		Entity* orbParticles = ecs.New("Orb Particles");
		transform = ecs.GetComponent<ComponentTransform>(orbParticles->ID());
		transform->SetPosition(glm::vec3(7.0f, 2.0f, -5.0f));
		ecs.AddComponent(orbParticles->ID(), ComponentParticleGenerator(resources->LoadTexture("Textures/Particles/orb.png", TEXTURE_DIFFUSE, false), 3, glm::vec3(0.0f), 1.0f, 1.0f, 0.5f, glm::vec3(2.5f)));
		ecs.GetComponent<ComponentParticleGenerator>(orbParticles->ID())->SetRandomParameters(energyParams);

		Entity* window = ecs.New("Window");
		transform = ecs.GetComponent<ComponentTransform>(window->ID());
		transform->SetPosition(glm::vec3(10.0f, 3.0f, 2.5f));
		ecs.AddComponent(window->ID(), ComponentGeometry(MODEL_PLANE));
		ecs.GetComponent<ComponentGeometry>(window->ID())->ApplyMaterialToModel(windowMaterial);

		Entity* leftWall = ecs.New("Left Wall");
		transform = ecs.GetComponent<ComponentTransform>(leftWall->ID());
		transform->SetPosition(glm::vec3(-11.0f, 0.25f, 0.0f));
		transform->SetScale(1.0f, 1.0f, 10.0f);
		ecs.AddComponent(leftWall->ID(), ComponentGeometry(MODEL_CUBE, false));
		ecs.AddComponent(leftWall->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(leftWall->ID())->IsMovedByCollisions(false);

		Entity* rightWall = ecs.New("Right Wall");
		transform = ecs.GetComponent<ComponentTransform>(rightWall->ID());
		transform->SetPosition(glm::vec3(11.0f, 0.25f, 0.0f));
		transform->SetScale(1.0f, 1.0f, 10.0f);
		ecs.AddComponent(rightWall->ID(), ComponentGeometry(MODEL_CUBE, false));
		ecs.AddComponent(rightWall->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(rightWall->ID())->IsMovedByCollisions(false);

		Entity* frontWall = ecs.New("Front Wall");
		transform = ecs.GetComponent<ComponentTransform>(frontWall->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.25f, -11.0f));
		transform->SetScale(12.0f, 1.0f, 1.0f);
		ecs.AddComponent(frontWall->ID(), ComponentGeometry(MODEL_CUBE, false));
		ecs.AddComponent(frontWall->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(frontWall->ID())->IsMovedByCollisions(false);

		Entity* backWall = ecs.New("Back Wall");
		transform = ecs.GetComponent<ComponentTransform>(backWall->ID());
		transform->SetPosition(glm::vec3(0.0f, 0.25f, 11.0f));
		transform->SetScale(12.0f, 1.0f, 1.0f);
		ecs.AddComponent(backWall->ID(), ComponentGeometry(MODEL_CUBE, false));
		ecs.AddComponent(backWall->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(backWall->ID())->IsMovedByCollisions(false);

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
		Entity* uiCanvas = ecs.New("Canvas");
		ecs.AddComponent(uiCanvas->ID(), ComponentUICanvas(SCREEN_SPACE));
		ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());
		canvas->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(25.0f, 135.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(25.0f, 10.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->AddUIElement(new UIText(std::string("Resolution: ") + std::to_string(SCR_WIDTH) + " X " + std::to_string(SCR_HEIGHT), glm::vec2(25.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));
		canvas->AddUIElement(new UIText(std::string("Shadow res: ") + std::to_string(renderManager->ShadowWidth()) + " X " + std::to_string(renderManager->ShadowHeight()), glm::vec2(25.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		const std::string renderPipeline = renderManager->GetRenderPipeline()->PipelineName();
		canvas->AddUIElement(new UIText(renderPipeline, glm::vec2(25.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		// Geometry debug UI
		UIBackground geometryDebugBackground;
		geometryDebugBackground.Colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		geometryDebugBackground.LeftRightUpDownExtents = glm::vec4(0.01f, 0.225f, 0.05f, 0.8f);
		geometryDebugBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		geometryDebugBackground.BorderThickness = 0.01f;
		geometryDebugBackground.Bordered = true;

		UIText* geoDebugText = new UIText(std::string("Geometry Debug"), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 135.0f), glm::vec2(0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		geoDebugText->SetBackground(geometryDebugBackground);
		geoDebugText->UseBackground(true);
		geoDebugText->SetActive(false);
		UIText* meshCountText = new UIText(std::string("Mesh count: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		meshCountText->SetActive(false);
		UIText* visibleCountText = new UIText(std::string("     - Visible: "), glm::vec2((SCR_WIDTH / 2.0f) - 145.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		visibleCountText->SetActive(false);
		UIText* bvhCountText = new UIText(std::string("BVHN count: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		bvhCountText->SetActive(false);
		UIText* aabbTestCountText = new UIText(std::string("AABB Tests: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 15.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
		aabbTestCountText->SetActive(false);
		canvas->AddUIElement(geoDebugText);
		canvas->AddUIElement(meshCountText);
		canvas->AddUIElement(visibleCountText);
		canvas->AddUIElement(bvhCountText);
		canvas->AddUIElement(aabbTestCountText);
#pragma endregion
	}

	void ParticleScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}