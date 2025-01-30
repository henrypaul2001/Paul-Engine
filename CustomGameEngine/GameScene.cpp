#include "GameScene.h"
#include <iostream>
#include <glm/vec3.hpp>
#include <glad/glad.h>
#include "GameInputManager.h"
#include "UIText.h"
namespace Engine
{
	GameScene::GameScene(SceneManager* sceneManager) : Scene(sceneManager, "GameScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
	}

	GameScene::~GameScene() {}

	void GameScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		CreateSystems();
		CreateEntities();
	}

	void GameScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			Entity* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void GameScene::keyDown(int key)
	{

	}

	void GameScene::CreateEntities()
	{
		/*
		Material* newMeshMaterial = new Material();
		newMeshMaterial->baseColour = glm::vec3(0.8f, 0.0f, 0.8f);
		newMeshMaterial->specular = glm::vec3(1.0f, 0.0f, 1.0f);
		newMeshMaterial->shininess = 100.0f;
		resources->AddMaterial("newMeshMaterial", newMeshMaterial);

		Material* blue = new Material();
		blue->baseColour = glm::vec3(0.0f, 0.0f, 0.8f);
		blue->specular = glm::vec3(0.0f, 0.0f, 1.0f);
		blue->shininess = 100.0f;
		resources->AddMaterial("blue", blue);

		Material* window = new Material();
		window->baseColour = glm::vec3(1.0f, 1.0f, 1.0f);
		window->specular = glm::vec3(0.8f, 0.0f, 0.0f);
		window->shininess = 60.0f;
		window->shadowCastAlphaDiscardThreshold = 0.5f;
		window->baseColourMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/window/window.png", TEXTURE_DIFFUSE, true));
		window->PushOpacityMap(ResourceManager::GetInstance()->LoadTexture("Materials/window/window_opacity.png", TEXTURE_OPACITY, false));
		resources->AddMaterial("window", window);

		Material* cobbleFloor = new Material();
		cobbleFloor->baseColourMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, true));
		cobbleFloor->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		cobbleFloor->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		cobbleFloor->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/displace.png", TEXTURE_DISPLACE, false));
		cobbleFloor->baseColour = glm::vec3(1.0f, 1.0f, 1.0f);
		cobbleFloor->specular = glm::vec3(0.5f, 0.5f, 0.5f);
		cobbleFloor->shininess = 60.0f;
		cobbleFloor->height_scale = -0.1f;
		resources->AddMaterial("cobbleFloor", cobbleFloor);

		Material* brickWall = new Material();
		brickWall->baseColourMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/diffuse.jpg", TEXTURE_DIFFUSE, true));
		brickWall->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/normal.jpg", TEXTURE_NORMAL, false));
		//brickWall->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/specular.jpg", TEXTURE_SPECULAR));
		brickWall->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/displace.jpg", TEXTURE_DISPLACE, false));
		brickWall->baseColour = glm::vec3(1.0f, 1.0f, 1.0f);
		brickWall->specular = glm::vec3(0.5f, 0.5f, 0.5f);
		brickWall->shininess = 60.0f;
		brickWall->height_scale = 0.1f;
		resources->AddMaterial("brickWall", brickWall);

		Entity* defaultCube = new Entity("Default Cube");
		defaultCube->AddComponent(new ComponentTransform(3.0f, -1.5f, 0.0f));
		//dynamic_cast<ComponentTransform*>(defaultCube->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(2.5f));
		defaultCube->AddComponent(new ComponentGeometry(MODEL_CUBE));
		//dynamic_cast<ComponentGeometry*>(defaultCube->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(window);
		entityManager->AddEntity(defaultCube);

		Entity* brickPlane = new Entity("Brick Wall");
		ComponentTransform* bricktransform = new ComponentTransform(0.0f, -5.0f, 0.0f);
		bricktransform->SetScale(glm::vec3(2.0f, 2.0f, 1.0f));
		bricktransform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		brickPlane->AddComponent(bricktransform);
		brickPlane->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(brickPlane->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(brickWall);
		entityManager->AddEntity(brickPlane);

		Entity* defaultPlane = new Entity("Default Plane");
		ComponentTransform* transform = new ComponentTransform(-6.0f, -5.0f, 0.0f);
		transform->SetScale(glm::vec3(2.0f, 2.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -25.0f);
		defaultPlane->AddComponent(transform);
		defaultPlane->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(defaultPlane->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(cobbleFloor);
		dynamic_cast<ComponentGeometry*>(defaultPlane->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		entityManager->AddEntity(defaultPlane);

		Entity* backpack = new Entity("Backpack");
		backpack->AddComponent(new ComponentTransform(0.0f, 2.0f, -2.0f));
		stbi_set_flip_vertically_on_load(true);
		backpack->AddComponent(new ComponentGeometry("Models/backpack/backpack.obj", false));
		stbi_set_flip_vertically_on_load(false);
		//dynamic_cast<ComponentGeometry*>(backpack->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(ResourceManager::GetInstance()->DefaultMaterial());
		entityManager->AddEntity(backpack);
		*/

		/*
		Entity* rock = new Entity("Rock");
		rock->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		rock->AddComponent(new ComponentVelocity(0.5f, 0.0f, 0.0f));
		rock->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		ComponentLight* rockLight = new ComponentLight(POINT);
		rockLight->Colour = glm::vec3(0.8f, 0.0f, 0.0f);
		rockLight->Specular = glm::vec3(0.8f, 0.0f, 0.0f);
		rockLight->Ambient = glm::vec3(0.2f, 0.0f, 0.0f);
		rock->AddComponent(rockLight);
		entityManager->AddEntity(rock);

		Entity* rockChild = new Entity("RockChild");
		rockChild->AddComponent(new ComponentTransform(0.0f, 3.5f, 0.0f));
		rockChild->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		dynamic_cast<ComponentTransform*>(rockChild->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));
		dynamic_cast<ComponentTransform*>(rockChild->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), 180.0f);
		dynamic_cast<ComponentTransform*>(rockChild->GetComponent(COMPONENT_TRANSFORM))->SetParent(rock);
		entityManager->AddEntity(rockChild);

		Entity* rockChild2 = new Entity("RockChild2");
		rockChild2->AddComponent(new ComponentTransform(0.0f, -10.0f, 0.0f));
		rockChild2->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		dynamic_cast<ComponentTransform*>(rockChild2->GetComponent(COMPONENT_TRANSFORM))->SetParent(rockChild);
		entityManager->AddEntity(rockChild2);

		Entity* rockChild3 = new Entity("RockChild3");
		rockChild3->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		rockChild3->AddComponent(new ComponentGeometry("Models/rock/rock.obj", false));
		dynamic_cast<ComponentTransform*>(rockChild3->GetComponent(COMPONENT_TRANSFORM))->SetParent(rockChild2);
		entityManager->AddEntity(rockChild3);
		*/

		Entity* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.Direction = glm::vec3(0.0f, -0.85f, -1.0f);
		directional.ShadowProjectionSize = 50.0f;
		ecs.AddComponent(dirLight->ID(), directional);

		/*
		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(0.0f, 3.0f, -9.0f));
		spotLight->AddComponent(new ComponentLight(SPOT));
		spotLight->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(blue);
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(0.0f, 0.0f, 1.0f);
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Specular = glm::vec3(0.0f, 0.0f, 1.0f);
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Ambient = glm::vec3(0.0f, 0.0f, 0.1f);
		dynamic_cast<ComponentLight*>(spotLight->GetComponent(COMPONENT_LIGHT))->Direction = glm::vec3(0.0f, -0.5f, 1.0f);
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(backpack);
		entityManager->AddEntity(spotLight);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(-8.0f, -3.0f, 2.0f));
		dynamic_cast<ComponentTransform*>(pointLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		pointLight->AddComponent(new ComponentLight(POINT));
		pointLight->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		pointLight->AddComponent(new ComponentVelocity(glm::vec3(1.0f, 0.0f, 0.0f)));
		//dynamic_cast<ComponentLight*>(pointLight->GetComponent(COMPONENT_LIGHT))->Colour = glm::vec3(0.0f, 0.0f, 1.0f);
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(backpack);
		entityManager->AddEntity(pointLight);

		Entity* testSphere = new Entity("Test Sphere");
		testSphere->AddComponent(new ComponentTransform(-2.0f, 0.0f, 3.0f));
		testSphere->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentGeometry*>(testSphere->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(window);
		dynamic_cast<ComponentGeometry*>(testSphere->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(5.0f);
		entityManager->AddEntity(testSphere);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -8.0f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(100.0f, 100.0f, 1.0f));
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		entityManager->AddEntity(floor);

		Entity* boxOne = new Entity("Box One");
		boxOne->AddComponent(new ComponentTransform(-2.5f, -7.5f, 10.0f));
		boxOne->AddComponent(new ComponentGeometry(MODEL_CUBE));
		boxOne->AddComponent(new ComponentVelocity(glm::vec3(0.0f, 0.0f, -1.0f)));
		dynamic_cast<ComponentTransform*>(boxOne->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		entityManager->AddEntity(boxOne);

		Entity* boxTwo = new Entity("Box Two");
		boxTwo->AddComponent(new ComponentTransform(-3.0f, -8.0f, 8.0f));
		boxTwo->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(boxTwo->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		entityManager->AddEntity(boxTwo);

		Entity* boxThree = new Entity("Box Three");
		boxThree->AddComponent(new ComponentTransform(-2.5f, -5.5f, 11.5f));
		boxThree->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(boxThree->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.75f));
		entityManager->AddEntity(boxThree);

		Entity* wall = new Entity("Wall");
		wall->AddComponent(new ComponentTransform(-3.0f, -6.0f, 15.0f));
		wall->AddComponent(new ComponentGeometry(MODEL_PLANE));
		wall->AddComponent(new ComponentVelocity(glm::vec3(1.0f, 0.0f, 0.0f)));
		entityManager->AddEntity(wall);

		Entity* spotShadowTest = new Entity("Spot light shadow test");
		//spotShadowTest->AddComponent(new ComponentTransform(2.5f, -7.1f, 9.5f));
		spotShadowTest->AddComponent(new ComponentTransform(-2.75f, -6.5f, 17.0f));
		spotShadowTest->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(spotShadowTest->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(spotShadowTest->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		ComponentLight* spot = new ComponentLight(SPOT);
		glm::vec3 lightPos = dynamic_cast<ComponentTransform*>(spotShadowTest->GetComponent(COMPONENT_TRANSFORM))->Position();
		glm::vec3 targetPos = dynamic_cast<ComponentTransform*>(boxTwo->GetComponent(COMPONENT_TRANSFORM))->Position();
		//spot->Direction = glm::normalize(lightPos - targetPos);
		spot->Direction = glm::vec3(0.0f, 0.0f, -1.0f);
		//spot->Cutoff = glm::radians(30.0f);
		spot->Colour = glm::vec3(0.0f, 1.0f, 0.0f);
		spot->Specular = glm::vec3(0.0f, 1.0f, 0.0f);
		spot->Ambient = glm::vec3(0.0f, 0.0f, 0.0f);
		spot->Linear = 0.045f;
		spot->Quadratic = 0.0075f;
		spotShadowTest->AddComponent(spot);
		entityManager->AddEntity(spotShadowTest);

		Entity* pointLightStressTest1 = new Entity("Point Light Stress Test 1");
		pointLightStressTest1->AddComponent(new ComponentTransform(-15.0f, -6.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressTest1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.35f));
		pointLightStressTest1->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(pointLightStressTest1->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		pointLightStressTest1->AddComponent(new ComponentLight(POINT));
		entityManager->AddEntity(pointLightStressTest1);

		Entity* pointLightStressShadowCaster1 = new Entity("Point Light Stress Shadow Caster 1");
		pointLightStressShadowCaster1->AddComponent(new ComponentTransform(-15.0f, -6.0f, 12.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressShadowCaster1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		pointLightStressShadowCaster1->AddComponent(new ComponentGeometry(MODEL_CUBE));
		entityManager->AddEntity(pointLightStressShadowCaster1);

		Entity* pointLightStressTest2 = new Entity("Point Light Stress Test 2");
		pointLightStressTest2->AddComponent(new ComponentTransform(25.0f, -5.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressTest2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.35f));
		pointLightStressTest2->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(pointLightStressTest2->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		pointLightStressTest2->AddComponent(new ComponentLight(POINT));
		entityManager->AddEntity(pointLightStressTest2);

		Entity* pointLightStressShadowCaster2 = new Entity("Point Light Stress Shadow Caster 2");
		pointLightStressShadowCaster2->AddComponent(new ComponentTransform(25.0f, -5.0f, 12.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressShadowCaster2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		pointLightStressShadowCaster2->AddComponent(new ComponentGeometry(MODEL_CUBE));
		entityManager->AddEntity(pointLightStressShadowCaster2);

		Entity* pointLightStressTest3 = new Entity("Point Light Stress Test 3");
		pointLightStressTest3->AddComponent(new ComponentTransform(6.0f, -5.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressTest3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.35f));
		pointLightStressTest3->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(pointLightStressTest3->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		pointLightStressTest3->AddComponent(new ComponentLight(POINT));
		entityManager->AddEntity(pointLightStressTest3);

		Entity* pointLightStressShadowCaster3 = new Entity("Point Light Stress Shadow Caster 3");
		pointLightStressShadowCaster3->AddComponent(new ComponentTransform(6.0f, -5.0f, 12.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressShadowCaster3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		pointLightStressShadowCaster3->AddComponent(new ComponentGeometry(MODEL_CUBE));
		entityManager->AddEntity(pointLightStressShadowCaster3);

		Entity* pointLightStressTest4 = new Entity("Point Light Stress Test 4");
		//pointLightStressTest4->AddComponent(new ComponentTransform(16.0f, -6.0f, 10.0f));
		pointLightStressTest4->AddComponent(new ComponentTransform(3.5f, -6.5f, 21.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressTest4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.35f));
		pointLightStressTest4->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(pointLightStressTest4->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		pointLightStressTest4->AddComponent(new ComponentLight(POINT));
		entityManager->AddEntity(pointLightStressTest4);

		Entity* pointLightStressShadowCaster4 = new Entity("Point Light Stress Shadow Caster 4");
		pointLightStressShadowCaster4->AddComponent(new ComponentTransform(16.0f, -6.0f, 12.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressShadowCaster4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		pointLightStressShadowCaster4->AddComponent(new ComponentGeometry(MODEL_CUBE));
		entityManager->AddEntity(pointLightStressShadowCaster4);

		Entity* pointLightStressTest5 = new Entity("Point Light Stress Test 5");
		pointLightStressTest5->AddComponent(new ComponentTransform(-26.0f, -6.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressTest5->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.35f));
		pointLightStressTest5->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(pointLightStressTest5->GetComponent(COMPONENT_GEOMETRY))->CastShadows(false);
		pointLightStressTest5->AddComponent(new ComponentLight(POINT));
		entityManager->AddEntity(pointLightStressTest5);

		Entity* pointLightStressShadowCaster5 = new Entity("Point Light Stress Shadow Caster 5");
		pointLightStressShadowCaster5->AddComponent(new ComponentTransform(-26.0f, -6.0f, 12.0f));
		dynamic_cast<ComponentTransform*>(pointLightStressShadowCaster5->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.65f));
		pointLightStressShadowCaster5->AddComponent(new ComponentGeometry(MODEL_CUBE));
		entityManager->AddEntity(pointLightStressShadowCaster5);

		Entity* ssaoCubeTest = new Entity("SSAO Cube Test");
		ssaoCubeTest->AddComponent(new ComponentTransform(1.75f, -7.0f, 20.75f));
		ssaoCubeTest->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(ssaoCubeTest->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		entityManager->AddEntity(ssaoCubeTest);

		Entity* ssaoCubeTest1 = new Entity("SSAO Cube Test 1");
		ssaoCubeTest1->AddComponent(new ComponentTransform(3.5f, -6.95f, 19.25f));
		ssaoCubeTest1->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(ssaoCubeTest1->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		entityManager->AddEntity(ssaoCubeTest1);

		Entity* window1 = new Entity("Window 1");
		window1->AddComponent(new ComponentTransform(10.0f, 3.0f, 2.5f));
		window1->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window1->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(window);
		entityManager->AddEntity(window1);

		Entity* window2 = new Entity("Window 2");
		window2->AddComponent(new ComponentTransform(9.0f, 3.0f, 1.5f));
		window2->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window2->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(window);
		entityManager->AddEntity(window2);

		Entity* window3 = new Entity("Window 3");
		window3->AddComponent(new ComponentTransform(10.0f, 3.0f, 0.5f));
		window3->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window3->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(window);
		entityManager->AddEntity(window3);

		Entity* window4 = new Entity("Window 4");
		window4->AddComponent(new ComponentTransform(9.5f, 2.5f, -0.5f));
		window4->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window4->GetComponent(COMPONENT_GEOMETRY))->ApplyMaterialToModel(window);
		entityManager->AddEntity(window4);
		*/

#pragma region UI
		TextFont* font = resources->LoadTextFont("Fonts/arial.ttf");
		Entity* uiCanvas = ecs.New("Canvas");
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

	void GameScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}

	void GameScene::ChangePostProcessEffect()
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

	void GameScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		float time = (float)glfwGetTime();
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Rock")->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(sin(time) * 1.0f, sin(time) * 1.0f, sin(time) * 1.0f));
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 0.0f, 1.0f), sin(time) * 180.0f);
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild2")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), sin(time) * 180.0f);
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild3")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0f, 1.0f, 1.0f), sin(time) * 180.0f);

		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Backpack")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 45.0f);

		//dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Point Light")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(1.0f, 0.0f, 0.0f) * sin(time) * 2.5f);

		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Brick Wall")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), time * -20.0f);

		//dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Wall")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(1.0f, 0.0f, 0.0f) * sin(time) * 3.5f);

		//dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Box One")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(0.0f, 0.0f, -1.0f) * sin(time) * 5.5f);

		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) { targetFPSPercentage = 1.0f; }

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		Entity* canvasEntity = ecs.Find("Canvas");
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

	void GameScene::Render()
	{
		Scene::Render();
	}

	void GameScene::Close()
	{
		std::cout << "Closing game scene" << std::endl;
	}
}