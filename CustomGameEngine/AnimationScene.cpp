#include "AnimationScene.h"
#include "GameInputManager.h"
#include "ComponentAnimator.h"
#include "SystemSkeletalAnimationUpdater.h"
#include "UIText.h"
#include "SystemUIRender.h"
namespace Engine {
	AnimationScene::AnimationScene(SceneManager* sceneManager) : Scene(sceneManager, "AnimationScene")
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
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			Entity* canvas = entityManager->FindEntity("Canvas");

			canvas->GetUICanvasComponent()->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->GetUICanvasComponent()->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
		if (key == GLFW_KEY_KP_3) {
			Entity* clone = entityManager->FindEntity("Swat")->Clone();
			clone->GetTransformComponent()->SetPosition(clone->GetTransformComponent()->GetWorldPosition() + glm::vec3(-5.0f, 0.0f, 0.0f));
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

		Entity* vampire = new Entity("Vampire");
		vampire->AddComponent(new ComponentTransform(originX, originY, originZ));
		vampire->AddComponent(new ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
		vampire->AddComponent(new ComponentAnimator(vampireDanceAnim));
		entityManager->AddEntity(vampire);

		int count = 1;
		for (int j = 0; j < xNum; j++) {
			for (int k = 0; k < zNum; k++) {
				if (count != 1) {
					Entity* clone = vampire->Clone();
					clone->GetTransformComponent()->SetPosition(glm::vec3(originX + (j * xDistance), originY, originZ + (k * zDistance)));
				}
				count++;
			}
		}

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

	void AnimationScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemSkeletalAnimationUpdater(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
	}
}