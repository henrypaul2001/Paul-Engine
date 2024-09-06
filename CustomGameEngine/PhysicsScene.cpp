#include "PhysicsScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
#include "SystemCollisionAABB.h"
#include "SystemCollisionSphere.h"
#include "SystemCollisionSphereAABB.h"
#include "ComponentCollisionBox.h"
#include "SystemCollisionBox.h"
#include "SystemCollisionBoxAABB.h"
#include "SystemCollisionSphereBox.h"
#include "ConstraintPosition.h"
#include "ConstraintRotation.h"
namespace Engine {
	PhysicsScene::PhysicsScene(SceneManager* sceneManager) : Scene(sceneManager, "PhysicsScene")
	{
		ballCount = 0;
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
	}

	PhysicsScene::~PhysicsScene()
	{

	}

	void PhysicsScene::ChangePostProcessEffect()
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

	void PhysicsScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);

		float time = (float)glfwGetTime();

		Entity* ball = entityManager->FindEntity("Physics Ball");
		//std::cout << "Velocity y = " << dynamic_cast<ComponentPhysics*>(ball->GetComponent(COMPONENT_PHYSICS))->Velocity().y << std::endl;

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

	void PhysicsScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void PhysicsScene::Close()
	{
	}

	void PhysicsScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void PhysicsScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		else if (key == GLFW_KEY_KP_8) {
			Entity* baseBall = entityManager->FindEntity("Ball");
			Entity* clonedBall = baseBall->Clone();
			clonedBall->GetTransformComponent()->SetPosition(glm::vec3(6.5f, 8.5f, -20.0f));
			clonedBall->GetPhysicsComponent()->SetVelocity(glm::vec3(0.0f));

			std::string name = clonedBall->Name();
			std::cout << "Creating " << name << std::endl;
		}
		else if (key == GLFW_KEY_F) {
			SystemPhysics* physics = dynamic_cast<SystemPhysics*>(systemManager->FindSystem(SYSTEM_PHYSICS, UPDATE_SYSTEMS));
			glm::vec3 axis = physics->GravityAxis();

			physics->Gravity(-axis);
		}
		else if (key == GLFW_KEY_KP_9) {
			Entity* torqueEntity = entityManager->FindEntity("Link 0");
			//torqueEntity->GetPhysicsComponent()->SetTorque(glm::vec3(0.0f, 2.0f, 0.0f));
			constraintManager->RemoveConstraint(constraintManager->GetConstraints().size() - 1);
		}
		else if (key == GLFW_KEY_KP_3) {
			Entity* cube = entityManager->FindEntity("Test Cube 2");
			cube->GetPhysicsComponent()->SetTorque(glm::vec3(5.0f, 0.0f, 0.0f));
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
	}

	void PhysicsScene::keyDown(int key)
	{

	}

	void PhysicsScene::CreateEntities()
	{
		ResourceManager* resources = ResourceManager::GetInstance();
		Material* textured = new Material();
		textured->baseColourMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, false));
		textured->specularMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		textured->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		textured->shininess = 5.0f;
		resources->AddMaterial("textured", textured);

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional->Colour = glm::vec3(1.0f);
		directional->Specular = glm::vec3(0.0f);
		directional->Direction = glm::vec3(0.01f, -0.9f, -0.01f);
		directional->MinShadowBias = 0.0f;
		directional->MaxShadowBias = 0.003f;
		directional->DirectionalLightDistance = 20.0f;
		directional->ShadowProjectionSize = 30.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* leftWall = new Entity("Left Wall");
		leftWall->AddComponent(new ComponentTransform(-6.15f, -20.0f, -20.0f));
		leftWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(leftWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(leftWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(-45.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		leftWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		dynamic_cast<ComponentCollisionBox*>(leftWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(leftWall);

		Entity* rightWall = new Entity("Right Wall");
		rightWall->AddComponent(new ComponentTransform(6.15f, -20.0f, -20.0f));
		rightWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(rightWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(rightWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(45.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		rightWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		dynamic_cast<ComponentCollisionBox*>(rightWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(rightWall);

		Entity* backWall = new Entity("Back Wall");
		backWall->AddComponent(new ComponentTransform(0.0f, -20.0f, -31.5f));
		backWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(backWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(backWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(glm::radians(88.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		backWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		dynamic_cast<ComponentCollisionBox*>(backWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(backWall);

		Entity* frontWall = new Entity("Front Wall");
		frontWall->AddComponent(new ComponentTransform(0.0f, -20.0f, -8.5f));
		frontWall->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(frontWall->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		dynamic_cast<ComponentTransform*>(frontWall->GetComponent(COMPONENT_TRANSFORM))->SetOrientation(glm::angleAxis(glm::radians(-88.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		//floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		frontWall->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		dynamic_cast<ComponentCollisionBox*>(frontWall->GetComponent(COMPONENT_COLLISION_BOX))->IsMovedByCollisions(false);
		entityManager->AddEntity(frontWall);

		Entity* physicsBall = new Entity("Physics Ball");
		physicsBall->AddComponent(new ComponentTransform(8.0f, 30.0f, -20.0f));
		physicsBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		physicsBall->AddComponent(new ComponentCollisionSphere(1.0f));
		physicsBall->AddComponent(new ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5
		entityManager->AddEntity(physicsBall);

		Entity* physicsBall2 = new Entity("Ball");
		physicsBall2->AddComponent(new ComponentTransform(6.5f, 8.5f, -20.0f));
		physicsBall2->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		physicsBall2->AddComponent(new ComponentCollisionSphere(1.0f));
		physicsBall2->AddComponent(new ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5
		dynamic_cast<ComponentCollisionSphere*>(physicsBall2->GetComponent(COMPONENT_COLLISION_SPHERE))->IsMovedByCollisions(true);
		entityManager->AddEntity(physicsBall2);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -2.0f, 0.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		dynamic_cast<ComponentCollisionAABB*>(floor->GetComponent(COMPONENT_COLLISION_AABB))->IsMovedByCollisions(false);
		entityManager->AddEntity(floor);

		Entity* inelasticBall = new Entity("Inelastic Ball");
		inelasticBall->AddComponent(new ComponentTransform(2.0f, 4.0f, -1.0f));
		inelasticBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		//dynamic_cast<ComponentGeometry*>(inelasticBall->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(textured);
		inelasticBall->AddComponent(new ComponentCollisionSphere(1.0f));
		inelasticBall->AddComponent(new ComponentPhysics(10.0f, 0.47f, 0.5f, 0.15f, true));
		entityManager->AddEntity(inelasticBall);

		Entity* elasticBall = new Entity("Elastic Ball");
		elasticBall->AddComponent(new ComponentTransform(2.0f, 8.0f, -1.0f));
		elasticBall->AddComponent(new ComponentGeometry(MODEL_SPHERE));
		dynamic_cast<ComponentTransform*>(elasticBall->GetComponent(COMPONENT_TRANSFORM))->SetScale(0.5f);
		//dynamic_cast<ComponentGeometry*>(elasticBall->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(textured);
		elasticBall->AddComponent(new ComponentCollisionSphere(1.0f));
		elasticBall->AddComponent(new ComponentPhysics(10.0f, 0.47f, 0.5f, 1.0f, true));
		entityManager->AddEntity(elasticBall);

		Entity* box = new Entity("Box");
		box->AddComponent(new ComponentTransform(-5.0f, 10.0f, -1.0f));
		//dynamic_cast<ComponentTransform*>(box->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.5f, 0.0f, 1.0f), 45.0f);
		box->AddComponent(new ComponentGeometry(MODEL_CUBE));
		box->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		box->AddComponent(new ComponentPhysics(5.0f, 1.05f, 2.0f, 0.7f, true, true));
		entityManager->AddEntity(box);

		//Entity* box2 = new Entity("Box 2");
		//box2->AddComponent(new ComponentTransform(-4.5f, 13.0f, -1.0f));
		//dynamic_cast<ComponentTransform*>(box2->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.5f, 0.0f, 1.0f), 45.0f);
		//box2->AddComponent(new ComponentGeometry(MODEL_CUBE));
		//box2->AddComponent(new ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, true));
		//box2->AddComponent(new ComponentPhysics(5.0f, 1.05f, 2.0f, 0.5f, true, true));
		//entityManager->AddEntity(box2);

		// Rope bridge
		glm::vec3 linkSize = glm::vec3(2.0f, 1.0f, 1.0f);
		float linkMass = 1.0f;
		int links = 15;
		float maxConstraintDistance = 0.0f;
		float linkDistance = -3.0f;
		float bias = 0.00000005f;

		glm::vec3 startPosition = glm::vec3(-25.0f, 5.0f, 17.0f);

		Entity* bridgeStart = new Entity("Bridge Start");
		bridgeStart->AddComponent(new ComponentTransform(startPosition));
		bridgeStart->GetTransformComponent()->SetScale(linkSize);
		bridgeStart->AddComponent(new ComponentGeometry(MODEL_CUBE));
		//bridgeStart->AddComponent(new ComponentPhysics(10000.0f, 1.05f, 2.0f, 0.7f, false, true));
		entityManager->AddEntity(bridgeStart);

		Entity* bridgeEnd = new Entity("Bridge End");
		bridgeEnd->AddComponent(new ComponentTransform(startPosition + glm::vec3(0.0f, 0.0f, (links + 1) * linkDistance)));
		bridgeEnd->GetTransformComponent()->SetScale(linkSize);
		bridgeEnd->AddComponent(new ComponentGeometry(MODEL_CUBE));
		//bridgeEnd->AddComponent(new ComponentPhysics(10000.0f, 1.05f, 2.0f, 0.7f, false, true));
		entityManager->AddEntity(bridgeEnd);

		Entity* previous = bridgeStart;
		for (int i = 0; i < links; i++) {
			std::string name = std::string("Link ") + std::string(std::to_string(i));
			Entity* newLink = new Entity(name);
			newLink->AddComponent(new ComponentTransform(startPosition + glm::vec3(0.0f, 0.0f, (i + 1) * linkDistance)));
			newLink->GetTransformComponent()->SetScale(linkSize);
			newLink->AddComponent(new ComponentPhysics(linkMass, 1.05f, 2.0f, 0.7f, true, true));
			newLink->AddComponent(new ComponentGeometry(MODEL_CUBE));
			entityManager->AddEntity(newLink);
			constraintManager->AddNewConstraint(new ConstraintPosition(*previous, *newLink, maxConstraintDistance, bias, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			previous = newLink;
		}
		constraintManager->AddNewConstraint(new ConstraintPosition(*previous, *bridgeEnd, maxConstraintDistance, bias, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

		Entity* testCube = new Entity("Test Cube");
		testCube->AddComponent(new ComponentTransform(0.0f, 10.0f, 15.0f));
		testCube->AddComponent(new ComponentGeometry(MODEL_CUBE));
		testCube->AddComponent(new ComponentPhysics(linkMass, 1.05f, 2.0f, 0.7f, false, true));
		//testCube->GetTransformComponent()->SetOrientation(glm::angleAxis(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.5f)));
		entityManager->AddEntity(testCube);

		Entity* testCube2 = new Entity("Test Cube 2");
		testCube2->AddComponent(new ComponentTransform(2.0f, 10.0f, 15.0f));
		testCube2->AddComponent(new ComponentGeometry(MODEL_CUBE));
		testCube2->AddComponent(new ComponentPhysics(linkMass, 1.05f, 2.0f, 0.7f, false, true));
		entityManager->AddEntity(testCube2);
		constraintManager->AddNewConstraint(new ConstraintRotation(*testCube, *testCube2, glm::vec3(20.0f, 0.0f, 0.0f)));

		Entity* aabbCube = new Entity("AABB Cube");
		aabbCube->AddComponent(new ComponentTransform(3.0f, 2.0f, 2.0f));
		aabbCube->AddComponent(new ComponentGeometry(MODEL_CUBE));
		aabbCube->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		aabbCube->AddComponent(new ComponentPhysics(30.0f, 0.47f, 1.0f, 0.5f, true, true));
		entityManager->AddEntity(aabbCube);

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

	void PhysicsScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphere(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBox(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionBoxAABB(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemCollisionSphereBox(entityManager, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddCollisionResponseSystem(new CollisionResolver(collisionManager));
		systemManager->AddConstraintSolver(new ConstraintSolver(constraintManager, 40));
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
	}
}