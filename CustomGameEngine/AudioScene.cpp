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

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));
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

		PBRMaterial* grass = new PBRMaterial();
		grass->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO, true));
		grass->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL, false));
		grass->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC, false));
		grass->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS, false));
		grass->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO, false));

		PBRMaterial* water = new PBRMaterial();
		water->albedo = glm::vec3(0.5f, 0.6f, 0.8f);
		water->metallic = 0.1f;
		water->roughness = 0.0f;
		water->ao = 1.0f;

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

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(30.0f, 80.0f), glm::vec2(0.25f, 0.25f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(30.0f, 30.0f), glm::vec2(0.20f, 0.20f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
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