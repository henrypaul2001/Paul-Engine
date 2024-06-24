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

		if (key == GLFW_KEY_KP_9) {
			ComponentAnimator* anim = entityManager->FindEntity("Swat")->GetAnimator();

			animIndex++;
			if (animIndex >= animations.size()) {
				animIndex = 0;
			}

			anim->ChangeAnimation(animations[animIndex]);
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
		directional->ShadowProjectionSize = 10.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		SkeletalAnimation* vampireDanceAnim = ResourceManager::GetInstance()->LoadAnimation("Models/vampire/dancing_vampire.dae");
		SkeletalAnimation* tripAnim = ResourceManager::GetInstance()->LoadAnimation("Animations/trip.dae");
		SkeletalAnimation* walkAnim = ResourceManager::GetInstance()->LoadAnimation("Animations/walk.dae");
		SkeletalAnimation* drunkWalk = ResourceManager::GetInstance()->LoadAnimation("Animations/drunkWalk.dae");
		SkeletalAnimation* injuredWalk = ResourceManager::GetInstance()->LoadAnimation("Animations/injuredWalk.dae");
		SkeletalAnimation* rifleWalk = ResourceManager::GetInstance()->LoadAnimation("Animations/rifleWalk.dae");
		SkeletalAnimation* sneakWalk = ResourceManager::GetInstance()->LoadAnimation("Animations/sneakWalk.dae");

		animIndex = 0;
		animations = { tripAnim, walkAnim, drunkWalk, injuredWalk, rifleWalk, sneakWalk };

		//SkeletalAnimation* nurseTestAnim = ResourceManager::GetInstance()->LoadAnimation("Models/nurseNew/nurse.dae");

		int xNum = 6;
		int zNum = 3;

		float originX = float(-xNum) / 2.0f;
		float originY = -0.5f;
		float originZ = float(-zNum) / 2.0f;

		float xDistance = 2.25f;
		float zDistance = -2.25f;

		int count = 0;
		for (int j = 0; j < xNum; j++) {
			for (int k = 0; k < zNum; k++) {
				std::string name = std::string("Vampire ") + std::string(std::to_string(count));

				Entity* vampire = new Entity(name);
				vampire->AddComponent(new ComponentTransform(originX + (j * xDistance), originY, originZ + (k * zDistance)));
				vampire->AddComponent(new ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
				vampire->AddComponent(new ComponentAnimator(vampireDanceAnim));
				entityManager->AddEntity(vampire);

				count++;
				std::cout << "vampire " << count << " created" << std::endl;
			}
		}

		//Entity* nurse = new Entity("Nurse");
		//nurse->AddComponent(new ComponentTransform(glm::vec3(-4.0f, -0.5f, 0.0f)));
		//nurse->AddComponent(new ComponentGeometry("Models/nurseNew/nurseMeshNew.dae", true));
		//nurse->AddComponent(new ComponentAnimator(nurseTestAnim));
		//nurse->GetTransformComponent()->SetScale(0.015f);
		//nurse->GetTransformComponent()->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
		//entityManager->AddEntity(nurse);

		//Entity* man = new Entity("Man");
		//man->AddComponent(new ComponentTransform(-4.0f, -0.5f, 3.0f));
		//man->AddComponent(new ComponentGeometry("Models/man/man.fbx", true));
		//man->AddComponent(new ComponentAnimator(gunRun));
		//man->GetTransformComponent()->SetScale(0.015f);
		//man->GetTransformComponent()->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		//entityManager->AddEntity(man);

		Entity* swat = new Entity("Swat");
		swat->AddComponent(new ComponentTransform(-4.0f, -0.5f, 0.0f));
		//swat->GetTransformComponent()->SetScale(0.015f);
		swat->AddComponent(new ComponentGeometry("Models/swat/swat.dae", false));
		swat->AddComponent(new ComponentAnimator(animations[animIndex]));
		entityManager->AddEntity(swat);

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