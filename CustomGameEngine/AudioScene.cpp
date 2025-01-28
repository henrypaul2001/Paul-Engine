#include "AudioScene.h"
#include "GameInputManager.h"
#include "UIText.h"
namespace Engine {
	AudioScene::AudioScene(SceneManager* sceneManager) : Scene(sceneManager, "AudioScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	AudioScene::~AudioScene() {}

	void AudioScene::ChangePostProcessEffect()
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

	void AudioScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		float time = (float)glfwGetTime();
		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) { targetFPSPercentage = 1.0f; }

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		EntityNew* canvasEntity = ecs.Find("Canvas");
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

	void AudioScene::Render()
	{
		Scene::Render();
	}

	void AudioScene::Close() {}

	void AudioScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();

		resources->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);
		renderManager->SetEnvironmentMap("Textures/Environment Maps/sky.hdr");
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
	}

	void AudioScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			EntityNew* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void AudioScene::keyDown(int key)
	{

	}

	void AudioScene::CreateEntities()
	{
		EntityNew* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Ambient = glm::vec3(0.1f, 0.1f, 0.1f);
		//directional.Colour = glm::vec3(1.0f, 1.0f, 1.5f) * 10.0f;
		directional.Colour = glm::vec3(0.5f, 0.5f, 0.5f);
		directional.Specular = glm::vec3(0.5f);
		directional.ShadowProjectionSize = 10.0f;
		directional.Far = 150.0f;
		ecs.AddComponent(dirLight->ID(), directional);

		PBRMaterial* grass = new PBRMaterial(glm::vec3(1.0f), 0.0f, 0.0f);
		grass->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO, true));
		grass->normalMaps.push_back(resources->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL, false));
		grass->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC, false));
		grass->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS, false));
		grass->aoMaps.push_back(resources->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO, false));
		resources->AddMaterial("Grass", grass);

		PBRMaterial* water = new PBRMaterial(glm::vec3(0.5f, 0.6f, 0.8f), 0.1f, 0.0f, 1.0f);
		resources->AddMaterial("Water", water);

		AudioFile* bell = resources->LoadAudio("Audio/bell.wav");
		AudioFile* waterAndNature = resources->LoadAudio("Audio/waterAndNature.wav");
		AudioFile* campfireCrackling = resources->LoadAudio("Audio/campfire.wav", 1.0f, 0.0f, 2.0f);

		EntityNew* floor = ecs.New("Floor");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(0.0f, -0.6f, 0.0f));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.GetComponent<ComponentGeometry>(floor->ID())->ApplyMaterialToModel(grass);
		ecs.GetComponent<ComponentGeometry>(floor->ID())->SetTextureScale(5.0f);
		transform->SetScale(20.0f, 0.1f, 20.0f);

		EntityNew* waterStream = ecs.New("WaterStream");
		transform = ecs.GetComponent<ComponentTransform>(waterStream->ID());
		transform->SetPosition(glm::vec3(8.0f, -0.45f, 0.0f));
		transform->SetScale(2.0f, 20.0f, 1.0f);
		transform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		ecs.AddComponent(waterStream->ID(), ComponentGeometry(MODEL_PLANE, true));
		ecs.GetComponent<ComponentGeometry>(waterStream->ID())->ApplyMaterialToModel(water);
		ecs.AddComponent(waterStream->ID(), ComponentAudioSource(waterAndNature));

		EntityNew* campfire = ecs.New("Campfire");
		transform = ecs.GetComponent<ComponentTransform>(campfire->ID());
		transform->SetPosition(glm::vec3(-9.0f, -0.1f, 12.0f));
		ecs.AddComponent(campfire->ID(), ComponentGeometry("Models/Campfire/PUSHILIN_campfire.obj", true));
		ComponentLight light = ComponentLight(POINT);
		light.CastShadows = false;
		light.Colour = glm::vec3(0.95f, 0.45f, 0.05f) * 50.0f;
		ecs.AddComponent(campfire->ID(), light);
		ecs.AddComponent(campfire->ID(), ComponentAudioSource(campfireCrackling));
		ecs.GetComponent<ComponentAudioSource>(campfire->ID())->GetSFXController()->enableWavesReverbSoundEffect(0.0f, -10.0f, 500.0f, 0.001f);
		const unsigned int campfireID = campfire->ID();

		EntityNew* fireLight = ecs.New("Fire Light");
		transform = ecs.GetComponent<ComponentTransform>(fireLight->ID());
		transform->SetPosition(glm::vec3(0.0f, 1.5f, 0.0f));
		light.CastShadows = true;
		light.Colour = glm::vec3(0.95f, 0.65f, 0.15f) * 50.0f;
		ecs.AddComponent(fireLight->ID(), light);
		ecs.GetComponent<ComponentTransform>(campfireID)->AddChild(fireLight->ID());

		EntityNew* campfireClone = ecs.Clone(campfireID);
		transform = ecs.GetComponent<ComponentTransform>(campfireClone->ID());
		transform->SetPosition(transform->GetWorldPosition() + glm::vec3(0.0f, 0.0f, -25.0f));

		EntityNew* boulder = ecs.New("Boulder");
		transform = ecs.GetComponent<ComponentTransform>(boulder->ID());
		transform->SetPosition(glm::vec3(-8.0f, 0.0f, 20.0f));
		transform->SetScale(10.0f);
		transform->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), 35.0f);
		ecs.AddComponent(boulder->ID(), ComponentGeometry("Models/Boulder/PUSHILIN_boulder.obj", true));

		EntityNew* caveWall = ecs.New("Cave wall");
		transform = ecs.GetComponent<ComponentTransform>(caveWall->ID());
		transform->SetPosition(glm::vec3(-15.0f, 0.0f, 13.0f));
		transform->SetScale(8.0f, 8.0f, 12.0f);
		transform->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), -35.0f);
		ecs.AddComponent(caveWall->ID(), ComponentGeometry("Models/rockTwo/model.obj", true));

		EntityNew* caveWallTwo = ecs.New("Cave wall 2");
		transform = ecs.GetComponent<ComponentTransform>(caveWallTwo->ID());
		transform->SetPosition(glm::vec3(-1.0f, 0.0f, 13.0f));
		transform->SetScale(8.0f, 8.0f, 12.0f);
		transform->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), 140.0f);
		ecs.AddComponent(caveWallTwo->ID(), ComponentGeometry("Models/rockTwo/model.obj", true));

		EntityNew* caveRoof = ecs.New("Cave roof");
		transform = ecs.GetComponent<ComponentTransform>(caveRoof->ID());
		transform->SetPosition(glm::vec3(-8.0f, 5.0f, 13.0f));
		transform->SetScale(11.0f, 3.0f, 8.0f);
		//transform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), 140.0f);
		ecs.AddComponent(caveRoof->ID(), ComponentGeometry("Models/rockTwo/model.obj", true));

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
		EntityNew* uiCanvas = ecs.New("Canvas");
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

	void AudioScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}