#include "PBRScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
#include "SystemUIRender.h"
#include "SystemUIMouseInteraction.h"
#include "UIText.h"
#include "UIImage.h"
#include "UITextButton.h"
#include "UIImageButton.h"
namespace Engine {
	PBRScene::PBRScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(400.0f);
		renderManager->GetRenderParams()->SetBloomPasses(10);
		renderManager->GetRenderParams()->SetSSAOSamples(32);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);
		renderManager->SetAdvBloomLensDirtTexture(ResourceManager::GetInstance()->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false));
	}

	PBRScene::~PBRScene()
	{

	}

	void PBRScene::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();

		renderManager->SetEnvironmentMap(ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/st_peters_square_night.hdr", true));
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
	}

	
	void ButtonEnter(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(1.0f, 1.0f, 1.0f));
	}

	void ButtonExit(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));
	}

	void ButtonPress(UIButton* button) 
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.15f, 0.7f, 0.15f));
	}

	void BloomBtnRelease(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		RenderParams* renderParams = RenderManager::GetInstance()->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();

		bool bloom = (renderOptions & RENDER_BLOOM) != 0;
		bool advancedBloom = (renderOptions & RENDER_ADVANCED_BLOOM) != 0;
		bool lensDirt = (renderOptions & RENDER_ADVANCED_BLOOM_LENS_DIRT) != 0;

		if (!bloom && !advancedBloom) {
			renderParams->EnableRenderOptions(RENDER_BLOOM);
			std::cout << "PBRSCENE::ENABLE::Bloom" << std::endl;
			textButton->SetText("Bloom: on");
			textButton->SetButtonScale(glm::vec2(260.0f, 50.0f));
		}
		else if (bloom && !advancedBloom) {
			renderParams->DisableRenderOptions(RENDER_BLOOM);
			renderParams->EnableRenderOptions(RENDER_ADVANCED_BLOOM);
			std::cout << "PBRSCENE::DISABLE::Bloom" << std::endl;
			std::cout << "PBRSCENE::ENABLE::Adv Bloom" << std::endl;
			textButton->SetText("Bloom: adv");
			textButton->SetButtonScale(glm::vec2(280.0f, 50.0f));
		}
		else if (advancedBloom && !lensDirt) {
			renderParams->EnableRenderOptions(RENDER_ADVANCED_BLOOM_LENS_DIRT);
			std::cout << "PBRSCENE::ENABLE::Adv Bloom Lens Dirt" << std::endl;
			textButton->SetText("Bloom: adv + dirt");
			textButton->SetButtonScale(glm::vec2(445.0f, 50.0f));
		}
		else if (advancedBloom && lensDirt) {
			renderParams->DisableRenderOptions(RENDER_BLOOM | RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);
			std::cout << "PBRSCENE::DISABLE::Bloom" << std::endl;
			std::cout << "PBRSCENE::DISABLE::Adv Bloom" << std::endl;
			std::cout << "PBRSCENE::DISABLE::Adv Bloom Lens Dirt" << std::endl;
			textButton->SetText("Bloom: off");
			textButton->SetButtonScale(glm::vec2(260.0f, 50.0f));
		}
	}

	void SSAOBtnRelease(UIButton* button) {
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		RenderParams* renderParams = RenderManager::GetInstance()->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();

		if ((renderOptions & RENDER_SSAO) != 0) {
			renderParams->DisableRenderOptions(RENDER_SSAO);
			std::cout << "PBRSCENE::Disable::SSAO" << std::endl;
			textButton->SetText("SSAO: off");
		}
		else {
			renderParams->EnableRenderOptions(RENDER_SSAO);
			std::cout << "PBRSCENE::Enable::SSAO" << std::endl;
			textButton->SetText("SSAO: on");
		}
	}

	void PBRScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		//directional->Ambient = glm::vec3(0.01f, 0.01f, 0.05f);
		directional->Ambient = glm::vec3(0.035f, 0.035f, 0.08f);
		directional->Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		//directional->Colour = glm::vec3(0.0f);
		directional->Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional->MinShadowBias = 0.0f;
		directional->MaxShadowBias = 0.003f;
		directional->DirectionalLightDistance = 20.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

#pragma region materials
		PBRMaterial* bloomTest = new PBRMaterial();
		bloomTest->albedo = glm::vec3(1500.0f);
		bloomTest->metallic = 0.0f;
		bloomTest->roughness = 0.0f;
		bloomTest->ao = 0.0f;

		PBRMaterial* gold = new PBRMaterial();
		gold->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));

		PBRMaterial* rusted_iron = new PBRMaterial();
		rusted_iron->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/albedo.png", TEXTURE_ALBEDO, true));
		rusted_iron->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/normal.png", TEXTURE_NORMAL, false));
		rusted_iron->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/metallic.png", TEXTURE_METALLIC, false));
		rusted_iron->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/roughness.png", TEXTURE_ROUGHNESS, false));
		rusted_iron->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/ao.png", TEXTURE_AO, false));

		PBRMaterial* plastic = new PBRMaterial();
		plastic->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/albedo.png", TEXTURE_ALBEDO, true));
		plastic->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/normal.png", TEXTURE_NORMAL, false));
		plastic->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/metallic.png", TEXTURE_METALLIC, false));
		plastic->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/roughness.png", TEXTURE_ROUGHNESS, false));
		plastic->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/ao.png", TEXTURE_AO, false));

		PBRMaterial* bricks = new PBRMaterial();
		bricks->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
		bricks->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
		bricks->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
		bricks->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
		bricks->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
		bricks->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
		bricks->height_scale = -0.1;

		PBRMaterial* grass = new PBRMaterial();
		grass->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO, true));
		grass->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL, false));
		grass->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC, false));
		grass->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS, false));
		grass->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO, false));

		PBRMaterial* leather = new PBRMaterial();
		leather->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/albedo.png", TEXTURE_ALBEDO, true));
		leather->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/normal.png", TEXTURE_NORMAL, false));
		leather->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/roughness.png", TEXTURE_ROUGHNESS, false));

		PBRMaterial* metal_plate = new PBRMaterial();
		metal_plate->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/albedo.png", TEXTURE_ALBEDO, true));
		metal_plate->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/normal.png", TEXTURE_NORMAL, false));
		metal_plate->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/metallic.png", TEXTURE_METALLIC, false));
		metal_plate->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/roughness.png", TEXTURE_ROUGHNESS, false));
		metal_plate->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/ao.png", TEXTURE_AO, false));
		metal_plate->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/displacement.png", TEXTURE_DISPLACE, false));
		metal_plate->height_scale = -0.1;

		PBRMaterial* scifi = new PBRMaterial();
		scifi->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/albedo.png", TEXTURE_ALBEDO, true));
		scifi->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/normal.png", TEXTURE_NORMAL, false));
		scifi->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/metallic.png", TEXTURE_METALLIC, false));
		scifi->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/roughness.png", TEXTURE_ROUGHNESS, false));
		scifi->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/ao.png", TEXTURE_AO, false));
		//scifi->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/displacement.png", TEXTURE_DISPLACE));
		//scifi->height_scale = -0.1;

		PBRMaterial* snow = new PBRMaterial();
		snow->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/albedo.png", TEXTURE_ALBEDO, true));
		snow->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/normal.png", TEXTURE_NORMAL, false));
		snow->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/specular.png", TEXTURE_METALLIC, false));
		snow->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/roughness.png", TEXTURE_ROUGHNESS, false));
		snow->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/ao.png", TEXTURE_AO, false));
		snow->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/displacement.png", TEXTURE_DISPLACE, false));
		snow->height_scale = -0.1;

		PBRMaterial* space_blanket = new PBRMaterial();
		space_blanket->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/albedo.png", TEXTURE_ALBEDO, true));
		space_blanket->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/normal.png", TEXTURE_NORMAL, false));
		space_blanket->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/metallic.png", TEXTURE_METALLIC, false));
		space_blanket->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/roughness.png", TEXTURE_ROUGHNESS, false));
		space_blanket->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/ao.png", TEXTURE_AO, false));

		PBRMaterial* wall = new PBRMaterial();
		wall->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/albedo.png", TEXTURE_ALBEDO, false));
		wall->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/normal.png", TEXTURE_NORMAL, false));
		wall->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/metallic.png", TEXTURE_METALLIC, false));
		wall->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/roughness.png", TEXTURE_ROUGHNESS, false));
		wall->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/ao.png", TEXTURE_AO, false));

		PBRMaterial* worn_corrugated_iron = new PBRMaterial();
		worn_corrugated_iron->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/albedo.png", TEXTURE_ALBEDO, true));
		worn_corrugated_iron->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/normal.png", TEXTURE_NORMAL, false));
		worn_corrugated_iron->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/roughness.png", TEXTURE_ROUGHNESS, false));
		worn_corrugated_iron->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/ao.png", TEXTURE_AO, false));
		worn_corrugated_iron->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/displacement.png", TEXTURE_DISPLACE, false));
		worn_corrugated_iron->height_scale = -0.1;

		PBRMaterial* earth = new PBRMaterial();
		earth->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/earth/albedo.jpg", TEXTURE_ALBEDO, true));
		earth->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/earth/displacement.jpg", TEXTURE_DISPLACE, false));
		earth->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/earth/specular.jpg", TEXTURE_METALLIC, false));
		earth->height_scale = -0.1;

		PBRMaterial* raindrops = new PBRMaterial();
		raindrops->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/albedo.jpg", TEXTURE_ALBEDO, true));
		raindrops->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/normal.png", TEXTURE_NORMAL, false));
		raindrops->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/roughness.jpg", TEXTURE_ROUGHNESS, false));
		raindrops->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/ao.jpg", TEXTURE_AO, false));
		raindrops->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/height.png", TEXTURE_DISPLACE, false));
		raindrops->opacityMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/opacity.png", TEXTURE_OPACITY, false));
		raindrops->isTransparent = true;
		raindrops->shadowCastAlphaDiscardThreshold = 1.0f;
#pragma endregion

#pragma region scene
		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.0f, 0.0));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bricks);
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(10.0f);
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(floor);

		Entity* rainFloor = new Entity("Rain Floor");
		rainFloor->AddComponent(new ComponentTransform(0.0f, -0.99f, 0.0f));
		rainFloor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		rainFloor->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(raindrops);
		rainFloor->GetGeometryComponent()->SetTextureScale(10.0f);
		rainFloor->GetTransformComponent()->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		rainFloor->GetTransformComponent()->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(rainFloor);

		Entity* wall1 = new Entity("Wall 1");
		wall1->AddComponent(new ComponentTransform(0.0f, 0.0f, 10.0f));
		wall1->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(snow);
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 180.0f);
		entityManager->AddEntity(wall1);

		Entity* wall2 = new Entity("Wall 2");
		wall2->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		wall2->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(space_blanket);
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		entityManager->AddEntity(wall2);

		Entity* wall3 = new Entity("Wall 3");
		wall3->AddComponent(new ComponentTransform(10.0f, 0.0f, 0.0f));
		wall3->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), -90.0f);
		entityManager->AddEntity(wall3);

		Entity* wall4 = new Entity("Wall 4");
		wall4->AddComponent(new ComponentTransform(-10.0f, 0.0f, 0.0f));
		wall4->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(scifi);
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 90.0f);
		entityManager->AddEntity(wall4);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(6.5f, 4.0f, -6.5f));
		pointLight->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light = new ComponentLight(POINT);
		light->Colour = glm::vec3(50.0, 50.0, 50.0);
		//light->Colour = glm::vec3(1.0, 1.0, 1.0);
		light->CastShadows = true;
		light->Active = true;
		pointLight->AddComponent(light);
		entityManager->AddEntity(pointLight);

		Entity* pointLight2 = new Entity("Point Light2");
		pointLight2->AddComponent(new ComponentTransform(-8.5f, 4.0f, 8.5f));
		pointLight2->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light2 = new ComponentLight(POINT);
		light2->Colour = glm::vec3(150.0, 150.0, 150.0);
		//light2->Colour = glm::vec3(1.0, 1.0, 1.0);
		light2->CastShadows = true;
		light2->Active = true;
		pointLight2->AddComponent(light2);
		entityManager->AddEntity(pointLight2);

		Entity* pointLight3 = new Entity("Point Light3");
		pointLight3->AddComponent(new ComponentTransform(6.5f, 4.0f, 6.5f));
		pointLight3->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light3 = new ComponentLight(POINT);
		light3->Colour = glm::vec3(50.0, 50.0, 50.0);
		//light3->Colour = glm::vec3(1.0, 1.0, 1.0);
		light3->CastShadows = true;
		light3->Active = true;
		pointLight3->AddComponent(light3);
		entityManager->AddEntity(pointLight3);

		Entity* pointLight4 = new Entity("Point Light4");
		pointLight4->AddComponent(new ComponentTransform(-6.5f, 2.0f, -6.5f));
		pointLight4->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light4 = new ComponentLight(POINT);
		light4->Colour = glm::vec3(50.0, 50.0, 50.0);
		//light4->Colour = glm::vec3(1.0, 1.0, 1.0);
		light4->CastShadows = true;
		light4->Active = true;
		pointLight4->AddComponent(light4);
		entityManager->AddEntity(pointLight4);

		Entity* pointLight5 = new Entity("Point Light5");
		pointLight5->AddComponent(new ComponentTransform(0.0f, 2.0f, 0.0f));
		pointLight5->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight5->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light5 = new ComponentLight(POINT);
		light5->Colour = glm::vec3(25.0f, 25.0f, 25.0f);
		//light5->Colour = glm::vec3(0.5, 0.5, 0.5);
		light5->Linear = 0.027f;
		light5->Quadratic = 0.0028f;
		light5->CastShadows = true;
		light5->Active = true;
		pointLight5->AddComponent(light5);
		entityManager->AddEntity(pointLight5);

		Entity* spotParent = new Entity("Spot Parent");
		spotParent->AddComponent(new ComponentTransform(1.0f, 0.0f, 4.6f));
		spotParent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(raindrops);
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(spotParent->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(1.0f));
		entityManager->AddEntity(spotParent);

		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(-5.5f, 1.0f, 0.0f));
		spotLight->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(spotParent);
		ComponentLight* spot = new ComponentLight(SPOT);
		spot->Colour = glm::vec3(70.0f, 20.0f, 40.0f);
		//spot->Colour = glm::vec3(0.7f, 0.2f, 0.4f);
		spot->CastShadows = true;
		spot->Direction = glm::vec3(-1.0f, 0.0f, 0.0f);
		spot->Cutoff = glm::cos(glm::radians(20.0f));
		spot->OuterCutoff = glm::cos(glm::radians(32.0f));
		spotLight->AddComponent(spot);
		entityManager->AddEntity(spotLight);

		Entity* goblet = new Entity("Goblet");
		goblet->AddComponent(new ComponentTransform(0.0f, 0.35f, 0.0f));
		dynamic_cast<ComponentTransform*>(goblet->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(4.0f));
		goblet->AddComponent(new ComponentGeometry("Models/PBR/brass_goblet/brass_goblet.obj", true));
		entityManager->AddEntity(goblet);

		Entity* cart = new Entity("Cart");
		cart->AddComponent(new ComponentTransform(5.0f, -1.0f, 2.0f));
		dynamic_cast<ComponentTransform*>(cart->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(2.0f));
		cart->AddComponent(new ComponentGeometry("Models/PBR/cart/cart.obj", true));
		entityManager->AddEntity(cart);

		Entity* bloomCube = new Entity("Bloom Cube");
		bloomCube->AddComponent(new ComponentTransform(-2.5f, 0.35f, 2.5f));
		dynamic_cast<ComponentTransform*>(bloomCube->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		bloomCube->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		ComponentLight* bloomLight = new ComponentLight(POINT);
		bloomLight->Colour = glm::vec3(50.0f, 50.0f, 50.0f);
		//bloomLight->Colour = glm::vec3(0.5f, 0.5f, 0.5f);
		bloomLight->CastShadows = false;
		bloomCube->AddComponent(bloomLight);
		dynamic_cast<ComponentGeometry*>(bloomCube->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bloomTest);
		entityManager->AddEntity(bloomCube);
#pragma endregion

#pragma region ui
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(30.0f, 80.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(30.0f, 30.0f), glm::vec2(0.20f, 0.20f), font, glm::vec3(0.0f, 0.0f, 0.0f)));

		// Bloom button
		UITextButton* bloomBtn = new UITextButton(std::string("Bloom: adv + dirt"), glm::vec2(350.0f, 60.0f), glm::vec2(0.4f, 0.4f), glm::vec2(445.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f));
		bloomBtn->SetMouseEnterCallback(ButtonEnter);
		bloomBtn->SetMouseExitCallback(ButtonExit);
		bloomBtn->SetMouseDownCallback(ButtonPress);
		bloomBtn->SetMouseUpCallback(BloomBtnRelease);
		canvas->GetUICanvasComponent()->AddUIElement(bloomBtn);

		// SSAO button
		UITextButton* ssaoBtn = new UITextButton(std::string("SSAO: on"), glm::vec2(850.0f, 60.0f), glm::vec2(0.4f), glm::vec2(260.0f, 50.0f), font, glm::vec3(0.8f));
		ssaoBtn->SetMouseEnterCallback(ButtonEnter);
		ssaoBtn->SetMouseExitCallback(ButtonExit);
		ssaoBtn->SetMouseDownCallback(ButtonPress);
		ssaoBtn->SetMouseUpCallback(SSAOBtnRelease);
		canvas->GetUICanvasComponent()->AddUIElement(ssaoBtn);

		entityManager->AddEntity(canvas);
#pragma endregion

		Material* nonPBRMat = new Material();
		nonPBRMat->diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
		nonPBRMat->specular = glm::vec3(1.0f, 0.0f, 0.0f);
		nonPBRMat->shininess = 100.0f;

		Entity* nonPBRTest = new Entity("NON PBR TEST");
		nonPBRTest->AddComponent(new ComponentTransform(-5.0f, 0.35f, 2.5f));
		nonPBRTest->AddComponent(new ComponentGeometry(MODEL_CUBE));
		nonPBRTest->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(nonPBRMat);
		entityManager->AddEntity(nonPBRTest);
	}

	void PBRScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddCollisionResponseSystem(new CollisionResolver(collisionManager));
		systemManager->AddSystem(new SystemUIMouseInteraction(inputManager), UPDATE_SYSTEMS);
	}

	void PBRScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);
	
		float time = (float)glfwGetTime();

		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Spot Parent")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 25.0f);

		glm::vec3 axis = dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Spot Light")->GetComponent(COMPONENT_TRANSFORM))->RotationAxis();
		//axis.x += 0.3 * Scene::dt * sin(time);
		//axis.y *= 0.3 * Scene::dt * sin(time);
		//axis.z += 0.3 * Scene::dt * cos(time);
		axis = glm::normalize(axis);
		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Spot Light")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 50.0f);

		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) {
			targetFPSPercentage = 1.0f;
		}

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));
	}

	void PBRScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void PBRScene::Close()
	{
		std::cout << "Closing PBR scene" << std::endl;
	}

	void PBRScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
	}

	void PBRScene::keyDown(int key)
	{

	}

	void PBRScene::ChangePostProcessEffect()
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
}