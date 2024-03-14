#include "PBRScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
namespace Engine {
	PBRScene::PBRScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SSAO = true;
		SetupScene();
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
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
	}

	void PBRScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = false;
		directional->Ambient = glm::vec3(0.01f, 0.01f, 0.05f);
		directional->Colour = glm::vec3(1.9f, 2.1f, 2.5f);
		directional->Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		PBRMaterial* gold = new PBRMaterial();
		gold->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO));

		PBRMaterial* rusted_iron = new PBRMaterial();
		rusted_iron->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/albedo.png", TEXTURE_ALBEDO));
		rusted_iron->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/normal.png", TEXTURE_NORMAL));
		rusted_iron->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/metallic.png", TEXTURE_METALLIC));
		rusted_iron->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/roughness.png", TEXTURE_ROUGHNESS));
		rusted_iron->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/ao.png", TEXTURE_AO));

		PBRMaterial* plastic = new PBRMaterial();
		plastic->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/albedo.png", TEXTURE_ALBEDO));
		plastic->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/normal.png", TEXTURE_NORMAL));
		plastic->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/metallic.png", TEXTURE_METALLIC));
		plastic->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/roughness.png", TEXTURE_ROUGHNESS));
		plastic->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/ao.png", TEXTURE_AO));

		PBRMaterial* bricks = new PBRMaterial();
		bricks->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO));
		bricks->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL));
		bricks->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC));
		bricks->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS));
		bricks->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO));
		bricks->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE));
		bricks->height_scale = -0.1;

		PBRMaterial* grass = new PBRMaterial();
		grass->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO));
		grass->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL));
		grass->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC));
		grass->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS));
		grass->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO));

		PBRMaterial* leather = new PBRMaterial();
		leather->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/albedo.png", TEXTURE_ALBEDO));
		leather->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/normal.png", TEXTURE_NORMAL));
		leather->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/roughness.png", TEXTURE_ROUGHNESS));

		PBRMaterial* metal_plate = new PBRMaterial();
		metal_plate->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/albedo.png", TEXTURE_ALBEDO));
		metal_plate->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/normal.png", TEXTURE_NORMAL));
		metal_plate->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/metallic.png", TEXTURE_METALLIC));
		metal_plate->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/roughness.png", TEXTURE_ROUGHNESS));
		metal_plate->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/ao.png", TEXTURE_AO));
		metal_plate->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/displacement.png", TEXTURE_DISPLACE));
		metal_plate->height_scale = -0.1;

		PBRMaterial* scifi = new PBRMaterial();
		scifi->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/albedo.png", TEXTURE_ALBEDO));
		scifi->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/normal.png", TEXTURE_NORMAL));
		scifi->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/metallic.png", TEXTURE_METALLIC));
		scifi->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/roughness.png", TEXTURE_ROUGHNESS));
		scifi->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/ao.png", TEXTURE_AO));
		//scifi->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/displacement.png", TEXTURE_DISPLACE));
		//scifi->height_scale = -0.1;

		PBRMaterial* snow = new PBRMaterial();
		snow->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/albedo.png", TEXTURE_ALBEDO));
		snow->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/normal.png", TEXTURE_NORMAL));
		snow->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/specular.png", TEXTURE_METALLIC));
		snow->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/roughness.png", TEXTURE_ROUGHNESS));
		snow->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/ao.png", TEXTURE_AO));
		snow->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/displacement.png", TEXTURE_DISPLACE));
		snow->height_scale = -0.1;

		PBRMaterial* space_blanket = new PBRMaterial();
		space_blanket->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/albedo.png", TEXTURE_ALBEDO));
		space_blanket->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/normal.png", TEXTURE_NORMAL));
		space_blanket->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/metallic.png", TEXTURE_METALLIC));
		space_blanket->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/roughness.png", TEXTURE_ROUGHNESS));
		space_blanket->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/ao.png", TEXTURE_AO));

		PBRMaterial* wall = new PBRMaterial();
		wall->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/albedo.png", TEXTURE_ALBEDO));
		wall->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/normal.png", TEXTURE_NORMAL));
		wall->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/metallic.png", TEXTURE_METALLIC));
		wall->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/roughness.png", TEXTURE_ROUGHNESS));
		wall->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/ao.png", TEXTURE_AO));

		PBRMaterial* worn_corrugated_iron = new PBRMaterial();
		worn_corrugated_iron->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/albedo.png", TEXTURE_ALBEDO));
		worn_corrugated_iron->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/normal.png", TEXTURE_NORMAL));
		worn_corrugated_iron->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/roughness.png", TEXTURE_ROUGHNESS));
		worn_corrugated_iron->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/ao.png", TEXTURE_AO));
		worn_corrugated_iron->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/displacement.png", TEXTURE_DISPLACE));
		worn_corrugated_iron->height_scale = -0.1;

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.0f, 0.0));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bricks);
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(10.0f);
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(floor);

		Entity* wall1 = new Entity("Wall 1");
		wall1->AddComponent(new ComponentTransform(0.0f, 0.0f, 10.0f));
		wall1->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(snow);
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 180.0f);
		entityManager->AddEntity(wall1);

		Entity* wall2 = new Entity("Wall 2");
		wall2->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		wall2->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(space_blanket);
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentTransform*>(wall2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		entityManager->AddEntity(wall2);

		Entity* wall3 = new Entity("Wall 3");
		wall3->AddComponent(new ComponentTransform(10.0f, 0.0f, 0.0f));
		wall3->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), -90.0f);
		entityManager->AddEntity(wall3);

		Entity* wall4 = new Entity("Wall 4");
		wall4->AddComponent(new ComponentTransform(-10.0f, 0.0f, 0.0f));
		wall4->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(scifi);
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 90.0f);
		entityManager->AddEntity(wall4);

		Entity* roof = new Entity("Roof");
		roof->AddComponent(new ComponentTransform(0.0f, 5.0f, 0.0));
		roof->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(roof->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(roof->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), 90.0f);
		//entityManager->AddEntity(roof);

		Material* lightCubeMaterial = new Material();
		lightCubeMaterial->diffuse = glm::vec3(75.0f / 255.0f, 0.0f, 130.0f / 255.0f);
		lightCubeMaterial->specular = lightCubeMaterial->diffuse;
		lightCubeMaterial->shininess = 100.0f;

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(6.5f, 4.0f, -6.5f));
		pointLight->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light = new ComponentLight(POINT);
		light->Colour = glm::vec3(50.0, 50.0, 50.0);
		light->CastShadows = false;
		light->Active = true;
		pointLight->AddComponent(light);
		entityManager->AddEntity(pointLight);

		Entity* pointLight2 = new Entity("Point Light2");
		pointLight2->AddComponent(new ComponentTransform(-6.5f, 4.0f, 6.5f));
		pointLight2->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light2 = new ComponentLight(POINT);
		light2->Colour = glm::vec3(50.0, 50.0, 50.0);
		light2->CastShadows = false;
		light2->Active = true;
		pointLight2->AddComponent(light2);
		entityManager->AddEntity(pointLight2);

		Entity* pointLight3 = new Entity("Point Light3");
		pointLight3->AddComponent(new ComponentTransform(6.5f, 4.0f, 6.5f));
		pointLight3->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light3 = new ComponentLight(POINT);
		light3->Colour = glm::vec3(50.0, 50.0, 50.0);
		light3->CastShadows = false;
		light3->Active = true;
		pointLight3->AddComponent(light3);
		entityManager->AddEntity(pointLight3);

		Entity* pointLight4 = new Entity("Point Light4");
		pointLight4->AddComponent(new ComponentTransform(-6.5f, 2.0f, -6.5f));
		pointLight4->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light4 = new ComponentLight(POINT);
		light4->Colour = glm::vec3(50.0, 50.0, 50.0);
		light4->CastShadows = false;
		light4->Active = true;
		pointLight4->AddComponent(light4);
		entityManager->AddEntity(pointLight4);

		Entity* pointLight5 = new Entity("Point Light5");
		pointLight5->AddComponent(new ComponentTransform(0.0f, 2.0f, 0.0f));
		pointLight5->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(pointLight5->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light5 = new ComponentLight(POINT);
		light5->Colour = glm::vec3(25.0f, 25.0f, 25.0f);
		light5->Linear = 0.027f;
		light5->Quadratic = 0.0028f;
		light5->CastShadows = false;
		light5->Active = true;
		pointLight5->AddComponent(light5);
		entityManager->AddEntity(pointLight5);

		Entity* spotParent = new Entity("Spot Parent");
		spotParent->AddComponent(new ComponentTransform(1.0f, 0.0f, 4.6f));
		spotParent->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(spotParent->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(1.0f));
		entityManager->AddEntity(spotParent);

		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(-5.5f, 1.0f, 0.0f));
		spotLight->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->SetShader(ResourceManager::GetInstance()->DefaultLitPBR());
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(spotParent);
		ComponentLight* spot = new ComponentLight(SPOT);
		spot->Colour = glm::vec3(10.0f, 3.0f, 5.0f);
		spot->CastShadows = false;
		spot->Direction = glm::vec3(-1.0f, 0.0f, 0.0f);
		spot->Cutoff = glm::cos(glm::radians(20.0f));
		spot->OuterCutoff = glm::cos(glm::radians(32.0f));
		spotLight->AddComponent(spot);
		entityManager->AddEntity(spotLight);

		/*
		Entity* backpack = new Entity("Backpack"); // non pbr materials can be rendered in pbr scenes. However, the higher light values of pbr lighting look horrendous on them
		backpack->AddComponent(new ComponentTransform(0.0f, 0.0f, -4.0f));
		stbi_set_flip_vertically_on_load(true);
		backpack->AddComponent(new ComponentGeometry("Models/backpack/backpack.obj", false));
		stbi_set_flip_vertically_on_load(false);
		entityManager->AddEntity(backpack);
		*/

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
	}

	void PBRScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
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
		else if (key == GLFW_KEY_P) {
			ToggleSSAO();
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

	void PBRScene::ToggleSSAO()
	{
		SSAO = !SSAO;
		std::cout << "SSAO: " << SSAO << std::endl;
		ResourceManager::GetInstance()->DeferredLightingPass()->Use();
		ResourceManager::GetInstance()->DeferredLightingPass()->setBool("useSSAO", SSAO);
	}
}