#include "AudioScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
#include "AudioManager.h"
#include "SystemAudio.h"
namespace Engine {
	AudioScene::AudioScene(SceneManager* sceneManager) : Scene(sceneManager)
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
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(1.0f, 1.0f, 1.5f);
		directional->Specular = glm::vec3(0.0f);
		directional->ShadowProjectionSize = 10.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		AudioFile* bell = ResourceManager::GetInstance()->LoadAudio("Audio/bell.wav");

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -0.6f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->GetTransformComponent()->SetScale(10.0f, 0.1f, 10.0f);
		entityManager->AddEntity(floor);

		Entity* bellTest = new Entity("BellTest");
		bellTest->AddComponent(new ComponentTransform(-5.0f, 0.0f, -5.0f));
		bellTest->AddComponent(new ComponentGeometry(MODEL_CUBE));
		bellTest->GetTransformComponent()->SetScale(0.5f);
		bellTest->AddComponent(new ComponentAudioSource(bell));
		entityManager->AddEntity(bellTest);

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
	}
}