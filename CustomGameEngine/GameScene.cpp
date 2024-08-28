#include "GameScene.h"
#include <iostream>
#include <glm/vec3.hpp>
#include "SystemPhysics.h"
#include <glad/glad.h>
#include "EntityManager.h"
#include "SystemManager.h"
#include "GameInputManager.h"
namespace Engine
{
	GameScene::GameScene(SceneManager* sceneManager) : Scene(sceneManager, "GameScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
	}

	GameScene::~GameScene()
	{
		delete entityManager;
		delete systemManager;
		delete inputManager;
	}

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
	}

	void GameScene::keyDown(int key)
	{

	}

	void GameScene::CreateEntities()
	{
		Material* newMeshMaterial = new Material();
		newMeshMaterial->diffuse = glm::vec3(0.8f, 0.0f, 0.8f);
		newMeshMaterial->specular = glm::vec3(1.0f, 0.0f, 1.0f);
		newMeshMaterial->shininess = 100.0f;

		Material* blue = new Material();
		blue->diffuse = glm::vec3(0.0f, 0.0f, 0.8f);
		blue->specular = glm::vec3(0.0f, 0.0f, 1.0f);
		blue->shininess = 100.0f;

		Material* window = new Material();
		window->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		window->specular = glm::vec3(0.8f, 0.0f, 0.0f);
		window->shininess = 60.0f;
		window->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/window/window.png", TEXTURE_DIFFUSE, true));
		window->opacityMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/window/window_opacity.png", TEXTURE_OPACITY, false));
		window->isTransparent = true;

		Material* cobbleFloor = new Material();
		cobbleFloor->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, true));
		cobbleFloor->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		cobbleFloor->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		cobbleFloor->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/displace.png", TEXTURE_DISPLACE, false));
		cobbleFloor->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		cobbleFloor->specular = glm::vec3(0.5f, 0.5f, 0.5f);
		cobbleFloor->shininess = 60.0f;
		cobbleFloor->height_scale = -0.1f;

		Material* brickWall = new Material();
		brickWall->diffuseMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/diffuse.jpg", TEXTURE_DIFFUSE, true));
		brickWall->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/normal.jpg", TEXTURE_NORMAL, false));
		//brickWall->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/specular.jpg", TEXTURE_SPECULAR));
		brickWall->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/brick_wall/displace.jpg", TEXTURE_DISPLACE, false));
		brickWall->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
		brickWall->specular = glm::vec3(0.5f, 0.5f, 0.5f);
		brickWall->shininess = 60.0f;
		brickWall->height_scale = 0.1f;

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
		dynamic_cast<ComponentGeometry*>(brickPlane->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(brickWall);
		entityManager->AddEntity(brickPlane);

		Entity* defaultPlane = new Entity("Default Plane");
		ComponentTransform* transform = new ComponentTransform(-6.0f, -5.0f, 0.0f);
		transform->SetScale(glm::vec3(2.0f, 2.0f, 1.0f));
		transform->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), -25.0f);
		defaultPlane->AddComponent(transform);
		defaultPlane->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(defaultPlane->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(cobbleFloor);
		dynamic_cast<ComponentGeometry*>(defaultPlane->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		entityManager->AddEntity(defaultPlane);

		Entity* backpack = new Entity("Backpack");
		backpack->AddComponent(new ComponentTransform(0.0f, 2.0f, -2.0f));
		stbi_set_flip_vertically_on_load(true);
		backpack->AddComponent(new ComponentGeometry("Models/backpack/backpack.obj", false));
		stbi_set_flip_vertically_on_load(false);
		//dynamic_cast<ComponentGeometry*>(backpack->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(ResourceManager::GetInstance()->DefaultMaterial());
		entityManager->AddEntity(backpack);

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

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		dirLight->AddComponent(new ComponentLight(DIRECTIONAL));
		dynamic_cast<ComponentLight*>(dirLight->GetComponent(COMPONENT_LIGHT))->Direction = glm::vec3(0.0f, -0.85f, -1.0f);
		dynamic_cast<ComponentLight*>(dirLight->GetComponent(COMPONENT_LIGHT))->ShadowProjectionSize = 50.0f;
		entityManager->AddEntity(dirLight);

		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(0.0f, 3.0f, -9.0f));
		spotLight->AddComponent(new ComponentLight(SPOT));
		spotLight->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(blue);
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
		dynamic_cast<ComponentGeometry*>(testSphere->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(window);
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
		dynamic_cast<ComponentGeometry*>(window1->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(window);
		entityManager->AddEntity(window1);

		Entity* window2 = new Entity("Window 2");
		window2->AddComponent(new ComponentTransform(9.0f, 3.0f, 1.5f));
		window2->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(window);
		entityManager->AddEntity(window2);

		Entity* window3 = new Entity("Window 3");
		window3->AddComponent(new ComponentTransform(10.0f, 3.0f, 0.5f));
		window3->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(window);
		entityManager->AddEntity(window3);

		Entity* window4 = new Entity("Window 4");
		window4->AddComponent(new ComponentTransform(9.5f, 2.5f, -0.5f));
		window4->AddComponent(new ComponentGeometry(MODEL_PLANE));
		dynamic_cast<ComponentGeometry*>(window4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(window);
		entityManager->AddEntity(window4);
	}

	void GameScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
	}

	void GameScene::ChangePostProcessEffect()
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

	void GameScene::Update()
	{
		Scene::Update();

		systemManager->ActionUpdateSystems(entityManager);
		float time = (float)glfwGetTime();
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Rock")->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(sin(time) * 1.0f, sin(time) * 1.0f, sin(time) * 1.0f));
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 0.0f, 1.0f), sin(time) * 180.0f);
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild2")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), sin(time) * 180.0f);
		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("RockChild3")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0f, 1.0f, 1.0f), sin(time) * 180.0f);

		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Backpack")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 45.0f);

		dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Point Light")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(1.0f, 0.0f, 0.0f) * sin(time) * 2.5f);

		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Brick Wall")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0f, 0.0f, 0.0f), time * -20.0f);

		dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Wall")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(1.0f, 0.0f, 0.0f) * sin(time) * 3.5f);

		dynamic_cast<ComponentVelocity*>(entityManager->FindEntity("Box One")->GetComponent(COMPONENT_VELOCITY))->SetVelocity(glm::vec3(0.0f, 0.0f, -1.0f) * sin(time) * 5.5f);
	}

	void GameScene::Render()
	{
		Scene::Render();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// Render scene
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void GameScene::Close()
	{
		std::cout << "Closing game scene" << std::endl;
	}
}