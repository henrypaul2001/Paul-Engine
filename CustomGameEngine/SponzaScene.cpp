#include "SponzaScene.h"
#include "GameInputManager.h"
#include "SystemUIRender.h"
#include "SystemUIMouseInteraction.h"
#include "UIText.h"
#include "UIImage.h"
#include "UITextButton.h"
#include "UIImageButton.h"
namespace Engine {
	SponzaScene::SponzaScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		entityManager = new EntityManager();
		systemManager = new SystemManager();
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		renderManager = RenderManager::GetInstance();

		SetupScene();
		renderManager->GetRenderParams()->SetSSAOSamples(32);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);
		ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);

		renderManager->SetEnvironmentMap("Textures/Environment Maps/sky.hdr");
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
		ResourceManager::GetInstance()->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");
	}

	SponzaScene::~SponzaScene()
	{
		delete entityManager;
		delete systemManager;
		delete inputManager;
	}

	void SponzaScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void SponzaScene::keyUp(int key)
	{

	}

	void SponzaScene::keyDown(int key)
	{

	}

	void SponzaScene::CreateEntities()
	{
		PBRMaterial* gold = new PBRMaterial();
		gold->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		dirLight->AddComponent(new ComponentLight(DIRECTIONAL));
		dirLight->GetLightComponent()->Direction = glm::vec3(0.35f, -0.6f, 0.0f);
		dirLight->GetLightComponent()->Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		dirLight->GetLightComponent()->Ambient = glm::vec3(0.01f, 0.01f, 0.05f) * 0.0f;
		dirLight->GetLightComponent()->ShadowProjectionSize = 30.0f;
		entityManager->AddEntity(dirLight);

		ResourceManager* resources = ResourceManager::GetInstance();
		resources->SetModelLoaderTextureTranslation(TEXTURE_NORMAL, aiTextureType_NORMALS);
		resources->SetModelLoaderTextureTranslation(TEXTURE_METALLIC, aiTextureType_METALNESS);
		resources->SetModelLoaderTextureTranslation(TEXTURE_ROUGHNESS, aiTextureType_DIFFUSE_ROUGHNESS);
		resources->SetModelLoaderTextureTranslation(TEXTURE_AO, aiTextureType_AMBIENT_OCCLUSION);
		resources->SetModelLoaderTextureTranslation(TEXTURE_HEIGHT, aiTextureType_HEIGHT);

		Entity* sponza = new Entity("Sponza");
		sponza->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		sponza->AddComponent(new ComponentGeometry("Models/PBR/newSponza/base/NewSponza_Main_glTF_003.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));
		entityManager->AddEntity(sponza);

		Entity* curtains = new Entity("Curtains");
		curtains->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		curtains->AddComponent(new ComponentGeometry("Models/PBR/newSponza/curtains/NewSponza_Curtains_glTF.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));
		entityManager->AddEntity(curtains);

		Entity* ivy = new Entity("Ivy");
		ivy->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ivy->AddComponent(new ComponentGeometry("Models/PBR/newSponza/ivy/NewSponza_IvyGrowth_glTF.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));
		entityManager->AddEntity(ivy);

		Entity* trees = new Entity("Trees");
		trees->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		trees->AddComponent(new ComponentGeometry("Models/PBR/newSponza/trees/NewSponza_CypressTree_glTF.gltf", true, false, false, defaultAssimpPostProcess | aiProcess_PreTransformVertices));
		trees->GetGeometryComponent()->SetCulling(false, GL_BACK);
		trees->GetGeometryComponent()->GetModel()->meshes[0]->GetPBRMaterial()->useDiffuseAlphaAsOpacity = false;
		trees->GetGeometryComponent()->GetModel()->meshes[1]->GetPBRMaterial()->useDiffuseAlphaAsOpacity = true;
		trees->GetGeometryComponent()->GetModel()->meshes[2]->GetPBRMaterial()->useDiffuseAlphaAsOpacity = false;
		entityManager->AddEntity(trees);

		resources->ResetModelLoaderTextureTranslationsToDefault();

		Entity* pointLight1 = new Entity("Point Light 1");
		pointLight1->AddComponent(new ComponentTransform(0.0f, 2.0f, 0.0f));
		pointLight1->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		pointLight1->GetGeometryComponent()->ApplyMaterialToModel(gold);
		pointLight1->GetGeometryComponent()->CastShadows(false);
		pointLight1->GetTransformComponent()->SetScale(0.25f);
		pointLight1->AddComponent(new ComponentLight(POINT));
		pointLight1->GetLightComponent()->Colour = glm::vec3(224.0f, 222.0f, 164.0f) * 0.3f;
		pointLight1->GetLightComponent()->Ambient = glm::vec3(0.0f);
		pointLight1->GetLightComponent()->CastShadows = false;
		entityManager->AddEntity(pointLight1);

		Entity* pointLight2 = new Entity("Point Light 2");
		pointLight2->AddComponent(new ComponentTransform(-10.0f, 7.5f, 0.0f));
		pointLight2->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		pointLight2->GetGeometryComponent()->ApplyMaterialToModel(gold);
		pointLight2->GetGeometryComponent()->CastShadows(false);
		pointLight2->GetTransformComponent()->SetScale(0.25f);
		pointLight2->AddComponent(new ComponentLight(POINT));
		pointLight2->GetLightComponent()->Colour = glm::vec3(224.0f, 222.0f, 164.0f) * 0.7f;
		pointLight2->GetLightComponent()->Ambient = glm::vec3(0.0f);
		pointLight2->GetLightComponent()->CastShadows = false;
		entityManager->AddEntity(pointLight2);

		Entity* pointLight3 = new Entity("Point Light 3");
		pointLight3->AddComponent(new ComponentTransform(16.0f, 1.5f, -1.0f));
		pointLight3->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		pointLight3->GetGeometryComponent()->ApplyMaterialToModel(gold);
		pointLight3->GetGeometryComponent()->CastShadows(false);
		pointLight3->GetTransformComponent()->SetScale(0.25f);
		pointLight3->AddComponent(new ComponentLight(POINT));
		pointLight3->GetLightComponent()->Colour = glm::vec3(224.0f, 222.0f, 164.0f) * 0.7f;
		pointLight3->GetLightComponent()->Ambient = glm::vec3(0.0f);
		pointLight3->GetLightComponent()->CastShadows = false;
		entityManager->AddEntity(pointLight3);

		// UI
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
	}

	void SponzaScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
	}

	void SponzaScene::Update()
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

	void SponzaScene::Render()
	{
		Scene::Render();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// Render scene
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void SponzaScene::Close()
	{

	}
}