#include "IBLScene.h"
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

		resources->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");
		renderManager->GetRenderParams()->SetAdvBloomLensDirtMaskStrength(0.5f);

		resources->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);
		resources->LoadHDREnvironmentMap("Textures/Environment Maps/newport_loft.hdr", true);
		renderManager->SetEnvironmentMap("Textures/Environment Maps/sky.hdr");
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);

		//systemManager->BakeReflectionProbes(entityManager->Entities());
		renderManager->GetBakedData().LoadReflectionProbesFromFile();
	}

	IBLScene::~IBLScene()
	{

	}

	void IBLScene::CreateEntities()
	{
		Entity* dirLight = ecs.New("Directional Light");

		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		//directional.Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		directional.Colour = glm::vec3(0.0f);
		directional.Ambient = glm::vec3(0.0f);
		directional.Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional.MinShadowBias = 0.0f;
		directional.MaxShadowBias = 0.003f;
		directional.DirectionalLightDistance = 20.0f;
		ecs.AddComponent(dirLight->ID(), directional);

#pragma region Materials
		PBRMaterial* bricks = new PBRMaterial(glm::vec3(1.0f));
		bricks->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
		bricks->normalMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
		bricks->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
		bricks->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
		bricks->aoMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
		bricks->heightMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
		bricks->height_scale = -0.1;
		bricks->textureScaling = glm::vec2(5.0f, 50.0f);
		resources->AddMaterial("bricks", bricks);

		PBRMaterial* geoBallMaterial = new PBRMaterial(glm::vec3(0.5f, 0.5f, 0.65f), 1.0f, 0.0f, 1.0f);
		resources->AddMaterial("geoBallMaterial", geoBallMaterial);

		PBRMaterial* ballMaterial = new PBRMaterial(glm::vec3(0.0f), 0.1f, 0.0f, 1.0f);
		resources->AddMaterial("ballMaterial", ballMaterial);

		PBRMaterial* raindrops = new PBRMaterial(glm::vec3(1.0f));
		raindrops->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/albedo.jpg", TEXTURE_ALBEDO, true));
		raindrops->normalMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/normal.png", TEXTURE_NORMAL, false));
		raindrops->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/roughness.jpg", TEXTURE_ROUGHNESS, false));
		raindrops->aoMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/ao.jpg", TEXTURE_AO, false));
		raindrops->heightMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/height.png", TEXTURE_DISPLACE, false));
		raindrops->PushOpacityMap(resources->LoadTexture("Materials/PBR/rain_drops/opacity.png", TEXTURE_OPACITY, false));
		raindrops->shadowCastAlphaDiscardThreshold = 1.0f;
		raindrops->textureScaling = glm::vec2(5.0f, 50.0f);
		resources->AddMaterial("raindrops", raindrops);
#pragma endregion

#pragma region Scene
		Entity* scifiInterior = ecs.New("Scifi Interior");
		ecs.AddComponent(scifiInterior->ID(), ComponentGeometry("Models/PBR/scifiInterior/scifiInterior.obj", true));

		const std::vector<Mesh*>& meshes = ecs.GetComponent<ComponentGeometry>(scifiInterior->ID())->GetModel()->meshes;

		const std::vector<glm::vec2> textureScales = {
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
			meshes[i]->GetMaterial()->textureScaling = textureScales[i];
		}

		Entity* geoBall = ecs.New("Geo Ball");
		ecs.AddComponent(geoBall->ID(), ComponentGeometry("Models/PBR/scifiInterior/coolGeoSphere.obj", true));
		ComponentGeometry* geometry = ecs.GetComponent<ComponentGeometry>(geoBall->ID());
		geometry->SetIsIncludedInReflectionProbes(false);
		geometry->ApplyMaterialToModel(geoBallMaterial);

		Entity* ball = ecs.New("Ball");
		ecs.GetComponent<ComponentTransform>(ball->ID())->SetPosition(glm::vec3(0.0f, 8.0f, 0.0f));
		ecs.AddComponent(ball->ID(), ComponentGeometry(MODEL_SPHERE, true));
		geometry = ecs.GetComponent<ComponentGeometry>(ball->ID());
		geometry->SetIsIncludedInReflectionProbes(false);
		geometry->ApplyMaterialToModel(ballMaterial);

		Entity* light = ecs.New("Light");
		ecs.GetComponent<ComponentTransform>(light->ID())->SetPosition(glm::vec3(0.0f, 2.5f, 0.0f));
		ComponentLight pointLight = ComponentLight(POINT);
		pointLight.CastShadows = false;
		pointLight.Colour = glm::vec3(25.0f, 25.0f, 32.5f);
		pointLight.Ambient = glm::vec3(0.0f);
		ecs.AddComponent(light->ID(), pointLight);

		Entity* floor = ecs.New("Floor");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(40.0f, 0.0f, 0.0f));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_PLANE, true));
		transform->SetScale(glm::vec3(5.0f, 50.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		ecs.GetComponent<ComponentGeometry>(floor->ID())->ApplyMaterialToModel(bricks);

		Entity* rainFloor = ecs.New("Rain Floor");
		transform = ecs.GetComponent<ComponentTransform>(rainFloor->ID());
		transform->SetPosition(glm::vec3(40.0f, 0.01f, 0.0f));
		ecs.AddComponent(rainFloor->ID(), ComponentGeometry(MODEL_PLANE, true));
		transform->SetScale(glm::vec3(5.0f, 50.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		geometry = ecs.GetComponent<ComponentGeometry>(rainFloor->ID());
		geometry->ApplyMaterialToModel(raindrops);
		geometry->SetIsIncludedInReflectionProbes(false);
		geometry->CastShadows(false);

		Entity streetLight = *ecs.New("StreetLight");
		transform = ecs.GetComponent<ComponentTransform>(streetLight.ID());
		transform->SetPosition(glm::vec3(40.0f, 2.5f, 0.0f));
		ComponentLight streetPoint = ComponentLight(POINT);
		streetPoint.CastShadows = false;
		streetPoint.Colour = glm::vec3(200.0f);
		streetPoint.Ambient = glm::vec3(0.0f);
		streetPoint.Specular = streetPoint.Colour;
		ecs.AddComponent(streetLight.ID(), streetPoint);
		ecs.AddComponent(streetLight.ID(), ComponentGeometry(MODEL_SPHERE, true));
		transform->SetScale(0.25f);

		Entity street = *ecs.New("Street");
		transform = ecs.GetComponent<ComponentTransform>(street.ID());
		transform->SetPosition(glm::vec3(40.0f, 0.0f, 0.0f));
		ecs.AddComponent(street.ID(), ComponentGeometry("Models/simpleStreet/street.obj", true));

		Entity* streetClone = ecs.Clone(street.ID());
		ecs.GetComponent<ComponentTransform>(streetClone->ID())->SetPosition(glm::vec3(40.0f, 0.0f, -20.0f));

		Entity* streetCloneTwo = ecs.Clone(street.ID());
		ecs.GetComponent<ComponentTransform>(streetCloneTwo->ID())->SetPosition(glm::vec3(40.0f, 0.0f, -40.0f));

		Entity* streetCloneThree = ecs.Clone(street.ID());
		ecs.GetComponent<ComponentTransform>(streetCloneThree->ID())->SetPosition(glm::vec3(40.0f, 0.0f, 20.0f));

		Entity* streetCloneFour = ecs.Clone(street.ID());
		ecs.GetComponent<ComponentTransform>(streetCloneFour->ID())->SetPosition(glm::vec3(40.0f, 0.0f, 40.0f));

		Entity* streetLightClone = ecs.Clone(streetLight.ID());
		ecs.GetComponent<ComponentTransform>(streetLightClone->ID())->SetPosition(glm::vec3(42.5f, 2.5f, -20.0f));
		ComponentLight* lightComponent = ecs.GetComponent<ComponentLight>(streetLightClone->ID());
		lightComponent->Colour = glm::vec3(125.0f, 200.0f, 125.0f);
		lightComponent->Specular = lightComponent->Colour;
#pragma endregion

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
		
		//Entity probeDebug = *ecs.New("Probe Debug");
		//transform = ecs.GetComponent<ComponentTransform>(probeDebug.ID());
		//transform->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
		//ecs.AddComponent(probeDebug.ID(), ComponentGeometry(MODEL_SPHERE, true));
		//transform->SetScale(8.0f);

		//Entity* probeDebugClone = ecs.Clone(probeDebug);
		//transform = ecs.GetComponent<ComponentTransform>(probeDebugClone->ID());
		//transform->SetPosition(glm::vec3(40.0f, 2.5f, 0.0f));
		//transform->SetScale(30.0f);

		//Entity* probeDebugCloneTwo = ecs.Clone(probeDebug);
		//transform = ecs.GetComponent<ComponentTransform>(probeDebugCloneTwo->ID());
		//transform->SetPosition(glm::vec3(40.0f, 2.5f, -20.0f));
		//transform->SetScale(30.0f);

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
		soiRadii.push_back(25.0f);
		soiRadii.push_back(25.0f);
		soiRadii.push_back(25.0f);
		soiRadii.push_back(25.0f);
		soiRadii.push_back(25.0f);

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

		renderManager->GetBakedData().InitialiseReflectionProbes(positions, localBounds, soiRadii, nearClips, farClips, renderSkybox, name, 1024u);
	}

	void IBLScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
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

	void IBLScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		Entity* geoBall = ecs.Find("Geo Ball");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(geoBall->ID());
		const glm::quat& currentOrientation = transform->GetOrientation();
		const glm::quat rotationIncrement = glm::angleAxis(1.0f * Scene::dt, glm::vec3(0.0f, 1.0f, 0.0f));

		transform->SetOrientation(currentOrientation * rotationIncrement);

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

		systemManager.ActionSystems();
	}

	void IBLScene::Render()
	{
		Scene::Render();
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

	void IBLScene::keyDown(int key)
	{

	}
}