#include "AudioScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
#include "AudioManager.h"
#include "SystemAudio.h"
namespace Engine {
	AudioScene::AudioScene(SceneManager* sceneManager) : Scene(sceneManager, "AudioScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	AudioScene::~AudioScene()
	{

	}

	void AudioScene::ChangePostProcessEffect()
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

	void AudioScene::Update()
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

		Entity* canvas = entityManager->FindEntity("Canvas");
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		BVHTree* geometryBVH = collisionManager->GetBVHTree();
		SystemFrustumCulling* culling = dynamic_cast<SystemFrustumCulling*>(systemManager->FindSystem(SYSTEM_FRUSTUM_CULLING, UPDATE_SYSTEMS));

		unsigned int meshCount = culling->GetTotalMeshes();
		unsigned int visibleMeshes = culling->GetVisibleMeshes();
		unsigned int nodeCount = geometryBVH->GetNodeCount();
		unsigned int aabbTests = culling->GetTotalAABBTests();

		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[6])->SetText("Mesh count: " + std::to_string(meshCount));
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[7])->SetText("     - Visible: " + std::to_string(visibleMeshes));
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[8])->SetText("BVHN count: " + std::to_string(nodeCount));
		dynamic_cast<UIText*>(canvas->GetUICanvasComponent()->UIElements()[9])->SetText("AABB Tests: " + std::to_string(aabbTests));
	}

	void AudioScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void AudioScene::Close()
	{

	}

	void AudioScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();

		ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);
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
			Entity* canvas = entityManager->FindEntity("Canvas");

			canvas->GetUICanvasComponent()->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void AudioScene::keyDown(int key)
	{

	}

	void AudioScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.1f, 0.1f, 0.1f);
		//directional->Colour = glm::vec3(1.0f, 1.0f, 1.5f) * 10.0f;
		directional->Colour = glm::vec3(0.5f, 0.5f, 0.5f);
		directional->Specular = glm::vec3(0.5f);
		directional->ShadowProjectionSize = 10.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		PBRMaterial* grass = new PBRMaterial(glm::vec3(1.0f), 0.0f, 0.0f);
		grass->baseColourMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO, true));
		grass->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL, false));
		grass->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC, false));
		grass->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS, false));
		grass->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO, false));

		PBRMaterial* water = new PBRMaterial(glm::vec3(0.5f, 0.6f, 0.8f), 0.1f, 0.0f, 1.0f);

		AudioFile* bell = ResourceManager::GetInstance()->LoadAudio("Audio/bell.wav");
		AudioFile* waterAndNature = ResourceManager::GetInstance()->LoadAudio("Audio/waterAndNature.wav");
		AudioFile* campfireCrackling = ResourceManager::GetInstance()->LoadAudio("Audio/campfire.wav", 1.0f, 0.0f, 2.0f);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -0.6f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->GetGeometryComponent()->ApplyMaterialToModel(grass);
		floor->GetGeometryComponent()->SetTextureScale(5.0f);
		floor->GetTransformComponent()->SetScale(20.0f, 0.1f, 20.0f);
		entityManager->AddEntity(floor);

		Entity* waterStream = new Entity("WaterStream");
		waterStream->AddComponent(new ComponentTransform(8.0f, -0.45f, 0.0f));
		waterStream->GetTransformComponent()->SetScale(2.0f, 20.0f, 1.0f);
		waterStream->GetTransformComponent()->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		waterStream->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		waterStream->GetGeometryComponent()->ApplyMaterialToModel(water);
		waterStream->AddComponent(new ComponentAudioSource(waterAndNature));
		entityManager->AddEntity(waterStream);

		Entity* campfire = new Entity("Campfire");
		campfire->AddComponent(new ComponentTransform(-9.0f, -0.1f, 12.0f));
		campfire->AddComponent(new ComponentGeometry("Models/Campfire/PUSHILIN_campfire.obj", true));
		campfire->AddComponent(new ComponentLight(POINT));
		campfire->GetLightComponent()->Colour = glm::vec3(0.95f, 0.45f, 0.05f) * 50.0f;
		campfire->GetLightComponent()->CastShadows = false;
		campfire->AddComponent(new ComponentAudioSource(campfireCrackling));
		campfire->GetAudioComponent()->GetSFXController()->enableWavesReverbSoundEffect(0.0f, -10.0f, 500.0f, 0.001f);
		entityManager->AddEntity(campfire);

		Entity* fireLight = new Entity("Fire Light");
		fireLight->AddComponent(new ComponentTransform(0.0f, 1.5f, 0.0f));
		fireLight->AddComponent(new ComponentLight(POINT));
		fireLight->GetLightComponent()->Colour = glm::vec3(0.95f, 0.65f, 0.15f) * 50.0f;
		fireLight->GetLightComponent()->CastShadows = true;
		fireLight->GetTransformComponent()->SetParent(campfire);
		entityManager->AddEntity(fireLight);

		Entity* campfireClone = campfire->Clone();
		campfireClone->GetTransformComponent()->SetPosition(campfireClone->GetTransformComponent()->GetWorldPosition() + glm::vec3(0.0f, 0.0f, -25.0f));

		Entity* boulder = new Entity("Boulder");
		boulder->AddComponent(new ComponentTransform(-8.0f, 0.0f, 20.0f));
		boulder->GetTransformComponent()->SetScale(10.0f);
		boulder->GetTransformComponent()->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), 35.0f);
		boulder->AddComponent(new ComponentGeometry("Models/Boulder/PUSHILIN_boulder.obj", true));
		entityManager->AddEntity(boulder);

		Entity* caveWall = new Entity("Cave wall");
		caveWall->AddComponent(new ComponentTransform(-15.0f, 0.0f, 13.0f));
		caveWall->GetTransformComponent()->SetScale(8.0f, 8.0f, 12.0f);
		caveWall->GetTransformComponent()->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), -35.0f);
		caveWall->AddComponent(new ComponentGeometry("Models/rockTwo/model.obj", true));
		entityManager->AddEntity(caveWall);

		Entity* caveWallTwo = new Entity("Cave wall 2");
		caveWallTwo->AddComponent(new ComponentTransform(-1.0f, 0.0f, 13.0f));
		caveWallTwo->GetTransformComponent()->SetScale(8.0f, 8.0f, 12.0f);
		caveWallTwo->GetTransformComponent()->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), 140.0f);
		caveWallTwo->AddComponent(new ComponentGeometry("Models/rockTwo/model.obj", true));
		entityManager->AddEntity(caveWallTwo);

		Entity* caveRoof = new Entity("Cave roof");
		caveRoof->AddComponent(new ComponentTransform(-8.0f, 5.0f, 13.0f));
		caveRoof->GetTransformComponent()->SetScale(11.0f, 3.0f, 8.0f);
		//caveRoof->GetTransformComponent()->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), 140.0f);
		caveRoof->AddComponent(new ComponentGeometry("Models/rockTwo/model.obj", true));
		entityManager->AddEntity(caveRoof);

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
		canvas->GetUICanvasComponent()->AddUIElement(geoDebugText);
		canvas->GetUICanvasComponent()->AddUIElement(meshCountText);
		canvas->GetUICanvasComponent()->AddUIElement(visibleCountText);
		canvas->GetUICanvasComponent()->AddUIElement(bvhCountText);
		canvas->GetUICanvasComponent()->AddUIElement(aabbTestCountText);
		entityManager->AddEntity(canvas);
	}

	void AudioScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemAudio(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
	}
}