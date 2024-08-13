#include "IBLScene.h"
#include "UIText.h"
#include "SystemFrustumCulling.h"
namespace Engine {
	IBLScene::IBLScene(SceneManager* sceneManager) : Scene(sceneManager, "IBLScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(400.0f);
		renderManager->GetRenderParams()->SetBloomPasses(10);
		renderManager->GetRenderParams()->SetSSAOSamples(32);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL);

		ResourceManager::GetInstance()->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");
		renderManager->GetRenderParams()->SetAdvBloomLensDirtMaskStrength(0.5f);

		ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);
		ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/newport_loft.hdr", true);
		renderManager->SetEnvironmentMap("Textures/Environment Maps/sky.hdr");
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);

		systemManager->BakeReflectionProbes(entityManager->Entities());
		//renderManager->GetBakedData().LoadReflectionProbesFromFile();
	}

	IBLScene::~IBLScene()
	{

	}

	void IBLScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));

		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		//directional->Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		directional->Colour = glm::vec3(0.0f);
		directional->Ambient = glm::vec3(0.0f);
		directional->Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional->MinShadowBias = 0.0f;
		directional->MaxShadowBias = 0.003f;
		directional->DirectionalLightDistance = 20.0f;

		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

#pragma region Materials
		PBRMaterial* bricks = new PBRMaterial();
		bricks->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
		bricks->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
		bricks->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
		bricks->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
		bricks->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
		bricks->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
		bricks->height_scale = -0.1;
		bricks->textureScaling = glm::vec2(5.0f, 30.0f);

		PBRMaterial* geoBallMaterial = new PBRMaterial();
		geoBallMaterial->albedo = glm::vec3(0.5f, 0.5f, 0.65f);
		geoBallMaterial->ao = 1.0f;
		geoBallMaterial->roughness = 0.0f;
		geoBallMaterial->metallic = 1.0f;

		PBRMaterial* ballMaterial = new PBRMaterial();
		ballMaterial->albedo = glm::vec3(0.0f);
		ballMaterial->ao = 1.0f;
		ballMaterial->roughness = 0.0f;
		ballMaterial->metallic = 0.1f;

		PBRMaterial* raindrops = new PBRMaterial();
		raindrops->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/albedo.jpg", TEXTURE_ALBEDO, true));
		raindrops->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/normal.png", TEXTURE_NORMAL, false));
		raindrops->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/roughness.jpg", TEXTURE_ROUGHNESS, false));
		raindrops->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/ao.jpg", TEXTURE_AO, false));
		raindrops->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/height.png", TEXTURE_DISPLACE, false));
		raindrops->opacityMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/opacity.png", TEXTURE_OPACITY, false));
		raindrops->isTransparent = true;
		raindrops->shadowCastAlphaDiscardThreshold = 1.0f;
		raindrops->textureScaling = glm::vec2(5.0f, 30.0f);
#pragma endregion

#pragma region Scene
		Entity* scifiInterior = new Entity("Scifi Interior");
		scifiInterior->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		scifiInterior->AddComponent(new ComponentGeometry("Models/PBR/scifiInterior/scifiInterior.obj", true));

		std::vector<Mesh*> meshes = scifiInterior->GetGeometryComponent()->GetModel()->meshes;

		std::vector<glm::vec2> textureScales = {
			glm::vec2(10.0f),
			glm::vec2(5.0f),
			glm::vec2(5.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(5.0f),
			glm::vec2(5.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.0f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(10.4f, 11.1f),
			glm::vec2(2.1f, 0.2f),
			glm::vec2(2.1f, 0.2f),
			glm::vec2(2.1f, 0.2f),
			glm::vec2(2.1f, 0.2f),
			glm::vec2(10.0f),
			glm::vec2(1.2f, 1.0f),
			glm::vec2(1.0f),
			glm::vec2(10.0f),
		};

		for (int i = 0; i < meshes.size(); i++) {
			meshes[i]->GetPBRMaterial()->textureScaling = textureScales[i];
		}
		entityManager->AddEntity(scifiInterior);

		Entity* geoBall = new Entity("Geo Ball");
		geoBall->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		geoBall->AddComponent(new ComponentGeometry("Models/PBR/scifiInterior/coolGeoSphere.obj", true));
		geoBall->GetGeometryComponent()->SetIsIncludedInReflectionProbes(false);
		geoBall->GetGeometryComponent()->ApplyMaterialToModel(geoBallMaterial);
		entityManager->AddEntity(geoBall);

		//Entity* ball = new Entity("Ball");
		//ball->AddComponent(new ComponentTransform(0.0f, 8.0f, 0.0f));
		//ball->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		//ball->GetGeometryComponent()->SetIsIncludedInReflectionProbes(false);
		//ball->GetGeometryComponent()->ApplyMaterialToModel(ballMaterial);
		//entityManager->AddEntity(ball);

		Entity* light = new Entity("Light");
		light->AddComponent(new ComponentTransform(0.0f, 2.5f, 0.0f));
		ComponentLight* pointLight = new ComponentLight(POINT);
		pointLight->CastShadows = false;
		pointLight->Colour = glm::vec3(25.0f, 25.0f, 32.5f);
		pointLight->Ambient = glm::vec3(0.0f);
		light->AddComponent(pointLight);
		entityManager->AddEntity(light);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(40.0f, 0.0f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		floor->GetTransformComponent()->SetScale(glm::vec3(5.0f, 50.0f, 1.0f));
		floor->GetTransformComponent()->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		floor->GetGeometryComponent()->ApplyMaterialToModel(bricks);
		entityManager->AddEntity(floor);

		Entity* rainFloor = new Entity("Rain Floor");
		rainFloor->AddComponent(new ComponentTransform(40.0f, 0.01f, 0.0f));
		rainFloor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		rainFloor->GetTransformComponent()->SetScale(glm::vec3(5.0f, 50.0f, 1.0f));
		rainFloor->GetTransformComponent()->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		rainFloor->GetGeometryComponent()->ApplyMaterialToModel(raindrops);
		rainFloor->GetGeometryComponent()->SetIsIncludedInReflectionProbes(false);
		rainFloor->GetGeometryComponent()->CastShadows(false);
		entityManager->AddEntity(rainFloor);

		Entity* streetLight = new Entity("StreetLight");
		streetLight->AddComponent(new ComponentTransform(40.0f, 2.5f, 0.0f));
		ComponentLight* streetPoint = new ComponentLight(POINT);
		streetPoint->CastShadows = false;
		streetPoint->Colour = glm::vec3(200.0f);
		streetPoint->Ambient = glm::vec3(0.0f);
		streetPoint->Specular = streetPoint->Colour;
		streetLight->AddComponent(streetPoint);
		streetLight->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		streetLight->GetTransformComponent()->SetScale(0.25f);
		entityManager->AddEntity(streetLight);

		Entity* street = new Entity("Street");
		street->AddComponent(new ComponentTransform(40.0f, 0.0f, 0.0f));
		street->AddComponent(new ComponentGeometry("Models/simpleStreet/street.obj", true));
		entityManager->AddEntity(street);

		Entity* streetClone = street->Clone();
		streetClone->GetTransformComponent()->SetPosition(glm::vec3(40.0f, 0.0f, -20.0f));

		Entity* streetCloneTwo = street->Clone();
		streetCloneTwo->GetTransformComponent()->SetPosition(glm::vec3(40.0f, 0.0f, -40.0f));

		Entity* streetCloneThree = street->Clone();
		streetCloneThree->GetTransformComponent()->SetPosition(glm::vec3(40.0f, 0.0f, 20.0f));

		Entity* streetCloneFour = street->Clone();
		streetCloneFour->GetTransformComponent()->SetPosition(glm::vec3(40.0f, 0.0f, 40.0f));

		Entity* streetLightClone = streetLight->Clone();
		streetLightClone->GetTransformComponent()->SetPosition(glm::vec3(42.5f, 2.5f, -20.0f));
		streetLightClone->GetLightComponent()->Colour = glm::vec3(125.0f, 200.0f, 125.0f);
		streetLightClone->GetLightComponent()->Specular = streetLightClone->GetLightComponent()->Colour;
#pragma endregion

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(25.0f, 135.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(25.0f, 10.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Resolution: ") + std::to_string(SCR_WIDTH) + " X " + std::to_string(SCR_HEIGHT), glm::vec2(25.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Shadow res: ") + std::to_string(renderManager->ShadowWidth()) + " X " + std::to_string(renderManager->ShadowHeight()), glm::vec2(25.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		std::string renderPipeline = "DEFERRED";
		if (renderManager->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
			renderPipeline = "FORWARD";
		}
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("G Pipeline: ") + renderPipeline, glm::vec2(25.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		entityManager->AddEntity(canvas);
#pragma endregion

		//Entity* probeDebug = new Entity("Probe Debug");
		//probeDebug->AddComponent(new ComponentTransform(glm::vec3(0.0f, 5.0f, 0.0f)));
		//probeDebug->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		//probeDebug->GetTransformComponent()->SetScale(8.0f);
		//entityManager->AddEntity(probeDebug);

		//Entity* probeDebugClone = probeDebug->Clone();
		//probeDebugClone->GetTransformComponent()->SetPosition(glm::vec3(40.0f, 2.5f, 0.0f));
		//probeDebugClone->GetTransformComponent()->SetScale(30.0f);

		//Entity* probeDebugCloneTwo = probeDebug->Clone();
		//probeDebugCloneTwo->GetTransformComponent()->SetPosition(glm::vec3(40.0f, 2.5f, -20.0f));
		//probeDebugCloneTwo->GetTransformComponent()->SetScale(30.0f);

		// Reflection probes
		std::vector<glm::vec3> positions;
		positions.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
		positions.push_back(glm::vec3(40.0f, 2.5f, 0.0f));
		positions.push_back(glm::vec3(40.0f, 2.5f, -20.0f));
		positions.push_back(glm::vec3(40.0f, 2.5f, 20.0f));
		positions.push_back(glm::vec3(40.0f, 2.5f, -40.0f));
		positions.push_back(glm::vec3(40.0f, 2.5f, 40.0f));

		// Temporary values
		std::vector<AABBPoints> localBounds;
		localBounds.push_back(AABBPoints(-5.25f, -5.0f, -5.25f, 5.25f, 4.0f, 5.25f));
		localBounds.push_back(AABBPoints(-5.0f, -2.5f, -50.0f, 5.0f, 8.5f, 50.0f));
		localBounds.push_back(AABBPoints(-5.0f, -2.5f, -30.0f, 5.0f, 8.5f, 70.0f));
		localBounds.push_back(AABBPoints(-5.0f, -2.5f, -70.0f, 5.0f, 8.5f, 30.0f));
		localBounds.push_back(AABBPoints(-5.0f, -2.5f, -10.0f, 5.0f, 8.5f, 90.0f));
		localBounds.push_back(AABBPoints(-5.0f, -2.5f, -90.0f, 5.0f, 8.5f, 10.0f));

		std::vector<float> soiRadii;
		soiRadii.push_back(8.0f);
		soiRadii.push_back(40.0f);
		soiRadii.push_back(40.0f);
		soiRadii.push_back(40.0f);
		soiRadii.push_back(40.0f);
		soiRadii.push_back(40.0f);

		std::vector<unsigned int> faceRes;
		faceRes.push_back(512);
		faceRes.push_back(1024);
		faceRes.push_back(1024);
		faceRes.push_back(1024);
		faceRes.push_back(1024);
		faceRes.push_back(1024);

		std::vector<float> nearClips;
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);

		std::vector<float> farClips;
		farClips.push_back(8.0f);
		farClips.push_back(100.0f);
		farClips.push_back(100.0f);
		farClips.push_back(100.0f);
		farClips.push_back(100.0f);
		farClips.push_back(100.0f);

		std::vector<bool> renderSkybox;
		renderSkybox.push_back(false);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);

		RenderManager::GetInstance()->GetBakedData().InitialiseReflectionProbes(positions, localBounds, soiRadii, faceRes, nearClips, farClips, renderSkybox, name);
	}

	void IBLScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemFrustumCulling(camera), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		SystemReflectionBaking* reflectionSystem = new SystemReflectionBaking();
		reflectionSystem->SetActiveCamera(camera);
		systemManager->AddSystem(reflectionSystem, RENDER_SYSTEMS);
	}

	void IBLScene::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void IBLScene::ChangePostProcessEffect()
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

	void IBLScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);

		Entity* geoBall = entityManager->FindEntity("Geo Ball");
		glm::quat currentOrientation = geoBall->GetTransformComponent()->GetOrientation();
		glm::quat rotationIncrement = glm::angleAxis(1.0f * Scene::dt, glm::vec3(0.0f, 1.0f, 0.0f));

		geoBall->GetTransformComponent()->SetOrientation(currentOrientation * rotationIncrement);

		float time = (float)glfwGetTime();
		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) {
			targetFPSPercentage = 1.0f;
		}

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));
	}

	void IBLScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void IBLScene::Close()
	{
		std::cout << "Closing IBL scene" << std::endl;
	}

	void IBLScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
	}

	void IBLScene::keyDown(int key)
	{

	}
}