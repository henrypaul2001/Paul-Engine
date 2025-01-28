#include "AnimationScene.h"
#include "GameInputManager.h"
#include "UIText.h"
namespace Engine {
	AnimationScene::AnimationScene(SceneManager* sceneManager) : Scene(sceneManager, "AnimationScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
		rebuildBVHOnUpdate = true;
	}

	AnimationScene::~AnimationScene() {}

	void AnimationScene::ChangePostProcessEffect()
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

	void AnimationScene::Update()
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

	void AnimationScene::Render()
	{
		Scene::Render();
	}

	void AnimationScene::Close() {}

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
			ComponentAnimator* anim = ecs.GetComponent<ComponentAnimator>(ecs.Find("Swat")->ID());

			animIndex++;
			if (animIndex >= animations.size()) { animIndex = 0; }

			anim->ChangeAnimation(animations[animIndex]);
		}
		if (key == GLFW_KEY_KP_3) {
			EntityNew* clone = ecs.Clone(ecs.Find("Swat")->ID());
			ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(clone->ID());
			transform->SetPosition(transform->GetWorldPosition() + glm::vec3(-5.0f, 0.0f, 0.0f));
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

	void AnimationScene::keyDown(int key)
	{

	}

	void AnimationScene::CreateEntities()
	{
		EntityNew* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional.Colour = glm::vec3(1.0f, 1.0f, 1.5f);
		directional.Specular = glm::vec3(0.0f);
		directional.ShadowProjectionSize = 10.0f;
		directional.Far = 150.0f;
		ecs.AddComponent(dirLight->ID(), directional);

		SkeletalAnimation* vampireDanceAnim = resources->LoadAnimation("Models/vampire/dancing_vampire.dae");
		SkeletalAnimation* tripAnim = resources->LoadAnimation("Animations/trip.dae");
		SkeletalAnimation* walkAnim = resources->LoadAnimation("Animations/walk.dae");
		SkeletalAnimation* drunkWalk = resources->LoadAnimation("Animations/drunkWalk.dae");
		SkeletalAnimation* injuredWalk = resources->LoadAnimation("Animations/injuredWalk.dae");
		SkeletalAnimation* rifleWalk = resources->LoadAnimation("Animations/rifleWalk.dae");
		SkeletalAnimation* sneakWalk = resources->LoadAnimation("Animations/sneakWalk.dae");

		animIndex = 0;
		animations = { tripAnim, walkAnim, drunkWalk, injuredWalk, rifleWalk, sneakWalk };

		//SkeletalAnimation* nurseTestAnim = resources->LoadAnimation("Models/nurseNew/nurse.dae");

		int xNum = 6;
		int zNum = 3;

		float originX = float(-xNum) / 2.0f;
		float originY = -0.5f;
		float originZ = float(-zNum) / 2.0f;

		float xDistance = 2.25f;
		float zDistance = -2.25f;

		EntityNew* vampire = ecs.New("Vampire");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(vampire->ID());
		transform->SetPosition(glm::vec3(originX, originY, originZ));
		ecs.AddComponent(vampire->ID(), ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
		ecs.AddComponent(vampire->ID(), ComponentAnimator(vampireDanceAnim));

		const unsigned int vampireID = vampire->ID();

		int count = 1;
		for (int j = 0; j < xNum; j++) {
			for (int k = 0; k < zNum; k++) {
				if (count != 1) {
					EntityNew* clone = ecs.Clone(vampireID);
					ecs.GetComponent<ComponentTransform>(clone->ID())->SetPosition(glm::vec3(originX + (j * xDistance), originY, originZ + (k * zDistance)));
				}
				count++;
			}
		}

		EntityNew* swat = ecs.New("Swat");
		transform = ecs.GetComponent<ComponentTransform>(swat->ID());
		transform->SetPosition(glm::vec3(-4.0f, -0.5f, 0.0f));
		//transform->SetScale(0.015f);
		ecs.AddComponent(swat->ID(), ComponentGeometry("Models/swat/swat.dae", false));
		ecs.AddComponent(swat->ID(), ComponentAnimator(animations[animIndex]));
		
		EntityNew* floor = ecs.New("Floor");
		transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(0.0f, -0.6f, 0.0f));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE));
		transform->SetScale(10.0f, 0.1f, 10.0f);

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

	void AnimationScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}