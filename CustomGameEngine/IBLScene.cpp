#include "IBLScene.h"
#include "UIText.h"
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

		ResourceManager::GetInstance()->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");

		//systemManager->BakeReflectionProbes(entityManager->Entities());
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
		directional->Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		directional->Ambient = directional->Colour * 0.08f;
		directional->Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional->MinShadowBias = 0.0f;
		directional->MaxShadowBias = 0.003f;
		directional->DirectionalLightDistance = 20.0f;

		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

#pragma region Materials

#pragma endregion

#pragma region Scene
		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.0f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		floor->GetGeometryComponent()->SetTextureScale(10.0f);
		floor->GetTransformComponent()->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		floor->GetTransformComponent()->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(floor);

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

		scifiInterior->GetGeometryComponent()->GetModel();
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

		// Reflection probes
		std::vector<glm::vec3> positions;
		positions.push_back(glm::vec3(7.5f, 2.0f, 7.5f));

		// Temporary values
		std::vector<AABBPoints> localBounds;
		localBounds.push_back(AABBPoints(-17.5f, -3.0f, -5.0f, 2.5f, 3.0f, 2.5f));

		std::vector<float> soiRadii;
		soiRadii.push_back(7.0f);

		//RenderManager::GetInstance()->GetBakedData().InitialiseReflectionProbes(positions, localBounds, soiRadii, name);
	}

	void IBLScene::CreateSystems()
	{
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

		ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);

		renderManager->SetEnvironmentMap("Textures/Environment Maps/sky.hdr");
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
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