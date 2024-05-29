#include "AnimationScene.h"
#include "GameInputManager.h"
#include "ComponentAnimator.h"
#include "SystemSkeletalAnimationUpdater.h"
#include "UIText.h"
#include "SystemUIRender.h"
namespace Engine {
	AnimationScene::AnimationScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	AnimationScene::~AnimationScene()
	{

	}

	void AnimationScene::ChangePostProcessEffect()
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

	void AnimationScene::Update()
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

	void AnimationScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void AnimationScene::Close()
	{

	}

	void AnimationScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void AnimationScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}

		if (key == GLFW_KEY_KP_3) {
			ComponentAnimator* anim = dynamic_cast<ComponentAnimator*>(entityManager->FindEntity("Vampire")->GetComponent(COMPONENT_ANIMATOR));
			anim->SetPause(!anim->Paused());
		}
	}

	void AnimationScene::keyDown(int key)
	{

	}

	void AnimationScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(1.0f, 1.0f, 1.5f);
		directional->Specular = glm::vec3(0.0f);
		directional->ShadowProjectionSize = 70.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		SkeletalAnimation* testAnim = ResourceManager::GetInstance()->LoadAnimation("Models/vampire/dancing_vampire.dae");

		Entity* vampire = new Entity("Vampire");
		vampire->AddComponent(new ComponentTransform(glm::vec3(0.0f, -0.5f, 0.0f)));
		vampire->AddComponent(new ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
		vampire->AddComponent(new ComponentAnimator(testAnim));
		entityManager->AddEntity(vampire);

		Entity* vampire2 = new Entity("Vampire 2");
		vampire2->AddComponent(new ComponentTransform(glm::vec3(4.0f, -0.5f, 0.0f)));
		vampire2->AddComponent(new ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
		vampire2->AddComponent(new ComponentAnimator(testAnim));
		entityManager->AddEntity(vampire2);

		Entity* cube = new Entity("Cube");
		cube->AddComponent(new ComponentTransform(0.0f, 1.75f, 3.0f));
		cube->AddComponent(new ComponentGeometry(MODEL_CUBE));
		cube->GetTransformComponent()->SetScale(0.75f);
		entityManager->AddEntity(cube);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -0.6f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->GetTransformComponent()->SetScale(10.0f, 0.1f, 10.0f);
		entityManager->AddEntity(floor);

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(30.0f, 80.0f), glm::vec2(0.25f, 0.25f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(30.0f, 30.0f), glm::vec2(0.20f, 0.20f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		entityManager->AddEntity(canvas);
	}

	void AnimationScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemSkeletalAnimationUpdater(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
	}
}