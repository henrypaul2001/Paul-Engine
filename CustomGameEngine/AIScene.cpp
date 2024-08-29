#include "AIScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
#include "SystemPathfinding.h"
#include "IdleState.h"
#include "SystemStateMachineUpdater.h"
namespace Engine {
	AIScene::AIScene(SceneManager* sceneManager) : Scene(sceneManager, "AIScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
	}

	AIScene::~AIScene()
	{
		delete navGrid;
	}

	void AIScene::ChangePostProcessEffect()
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

	void AIScene::Update()
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
		
		// Player movement
		Entity* agent = entityManager->FindEntity("Agent");
		Entity* agentClone = entityManager->FindEntity("Agent (1)");
		Entity* target = entityManager->FindEntity("Target");

		float moveSpeed = 2.5f * Scene::dt;

		if (inputManager->IsKeyDown(GLFW_KEY_KP_6)) {
			// Move right
			target->GetTransformComponent()->SetPosition(target->GetTransformComponent()->GetWorldPosition() + glm::vec3(moveSpeed, 0.0f, 0.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_4)) {
			// Move left
			target->GetTransformComponent()->SetPosition(target->GetTransformComponent()->GetWorldPosition() + glm::vec3(-moveSpeed, 0.0f, 0.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_5)) {
			// Move down
			target->GetTransformComponent()->SetPosition(target->GetTransformComponent()->GetWorldPosition() + glm::vec3(0.0f, 0.0f, moveSpeed));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_8)) {
			// Move up
			target->GetTransformComponent()->SetPosition(target->GetTransformComponent()->GetWorldPosition() + glm::vec3(0.0f, 0.0f, -moveSpeed));
		}

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[10])->SetText("AI State: " + agent->GetStateController()->GetStateMachine().GetActiveStateName());
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[11])->SetText("AI State: " + agentClone->GetStateController()->GetStateMachine().GetActiveStateName());
	}

	void AIScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void AIScene::Close()
	{

	}

	void AIScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Nav grid
		navGrid = new NavigationGrid("Data/NavigationGrid/TestGrid7.txt");

		CreateSystems();
		CreateEntities();
	}

	void AIScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
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

	void AIScene::keyDown(int key)
	{
		
	}

	void AIScene::CreateEntities()
	{
		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		directional->Colour = glm::vec3(0.7f, 0.65f, 0.85f) * 2.0f;
		directional->Specular = directional->Colour;
		directional->Ambient = directional->Colour * 200.0f;
		directional->Far = 300.0f;
		directional->DirectionalLightDistance = 150.0;
		directional->ShadowProjectionSize = 100.0f;
		directional->CastShadows = false;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		PBRMaterial* agentMaterial = new PBRMaterial();
		agentMaterial->albedo = glm::vec3(100.0f);

		PBRMaterial* walkable = new PBRMaterial();
		walkable->albedo = glm::vec3(100.0f, 0.0f, 0.0f);

		PBRMaterial* path = new PBRMaterial();
		path->albedo = glm::vec3(0.0f, 0.0f, 100.0f);

		PBRMaterial* grass = new PBRMaterial();
		grass->albedo = glm::vec3(65.0f, 152.0f, 10.0f) / 100.0f;

		PBRMaterial* wallMaterial = new PBRMaterial();
		wallMaterial->albedo = glm::vec3(248.0f, 229.0f, 187.0f) / 100.0f;

		// Pathfinding debug grid
		Entity* baseInstanceWalkable = new Entity("Base Instance Non Walkable");
		baseInstanceWalkable->AddComponent(new ComponentTransform(0.0f, -10.0f, 0.0f));
		baseInstanceWalkable->AddComponent(new ComponentGeometry(MODEL_SPHERE, true, true));
		baseInstanceWalkable->GetGeometryComponent()->CastShadows(false);
		baseInstanceWalkable->GetGeometryComponent()->ApplyMaterialToModel(walkable);
		entityManager->AddEntity(baseInstanceWalkable);

		int xNum = navGrid->GetGridWidth();
		int zNum = navGrid->GetGridHeight();

		float originX = 0.0f;
		float originZ = 0.0f;

		float nodeSize = navGrid->GetNodeSize();

		std::vector<NavGridNode*> nodes = navGrid->GetNodes();

		bool drawNavGridDebug = false;
		if (drawNavGridDebug) {
			std::vector<Entity*> gridEntities;
			gridEntities.reserve((xNum / 3) * (zNum / 3));

			int count = 0;
			for (int j = 0; j < xNum; j++) {
				for (int k = 0; k < zNum; k++) {
					NavGridNode* currentNode = nodes[(xNum * k) + j];

					if (currentNode->type == '.') {

						std::string name = std::string("Sphere ") + std::string(std::to_string(count));
						Entity* sphere = new Entity(name);
						sphere->AddComponent(new ComponentTransform(currentNode->worldPosition.x, 0.5f, currentNode->worldPosition.z));
						sphere->GetTransformComponent()->SetScale(nodeSize / 2.0f);

						// Non walkable node
						baseInstanceWalkable->GetGeometryComponent()->AddNewInstanceSource(sphere);

						gridEntities.push_back(sphere);
						entityManager->AddEntity(sphere);
						std::cout << "Sphere " << count << " created" << std::endl;
					}
					count++;
				}
			}
		}

		// Construct scene based on navigation grid
		glm::vec3 floorScale = glm::vec3(((xNum - 1.0f) * nodeSize) / 2.0f, 0.5f, ((zNum - 1.0f) * nodeSize) / 2.0f);
		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(((xNum - 1.0f) * nodeSize) / 2.0f, 0.0f, ((zNum - 1.0f) * nodeSize) / 2.0f));
		floor->GetTransformComponent()->SetScale(floorScale);
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		floor->GetAABBCollisionComponent()->IsMovedByCollisions(false);
		floor->GetGeometryComponent()->ApplyMaterialToModel(grass);
		entityManager->AddEntity(floor);

		camera->SetPosition(glm::vec3(floor->GetTransformComponent()->GetWorldPosition().x, ((xNum + zNum) * nodeSize) - 20.0f, floor->GetTransformComponent()->GetWorldPosition().z + nodeSize * 3));
		camera->SetPitch(-89.0f);

		// Create walls
		int count = 0;
		for (int y = 0; y < zNum; y++) {
			for (int x = 0; x < xNum; x++) {
				NavGridNode* currentNode = nodes[(xNum * y) + x];

				if (currentNode->type == 'x') {
					// Wall

					// Check right extent
					NavGridNode* rightNode = currentNode;

					int index;

					int rightExtent = 0;
					while (rightNode != nullptr && rightNode->type == 'x') {
						rightExtent++;
						index = (xNum * (y)) + (x + rightExtent);
						if (index < nodes.size() && x + rightExtent < xNum) {
							rightNode = nodes[index];
						}
						else {
							break;
						}
					}

					// Check down extent
					NavGridNode* downNode = currentNode;

					int downExtent = 0;
					while (downNode != nullptr && downNode->type == 'x') {
						downExtent++;
						
						index = (xNum * (y + downExtent)) + x;
						if (index < nodes.size() && y + downExtent < zNum) {
							downNode = nodes[index];
						}
						else {
							break;
						}
					}

					if (rightExtent == 1 && downExtent == 1) {
						// Wall scales to right
						Entity* wall = new Entity(std::string("Wall ") + std::string(std::to_string(count)));
						wall->AddComponent(new ComponentTransform(currentNode->worldPosition.x, 1.5f, currentNode->worldPosition.z));
						wall->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
						wall->GetTransformComponent()->SetScale(glm::vec3(nodeSize / 2.0f, 1.5f, nodeSize / 2.0f));
						wall->GetGeometryComponent()->ApplyMaterialToModel(wallMaterial);
						entityManager->AddEntity(wall);
						std::cout << "Wall " << count << " created" << std::endl;

						count++;
					}
					else if (rightExtent > downExtent) {
						// Wall scales to right
						Entity* wall = new Entity(std::string("Wall ") + std::string(std::to_string(count)));
						wall->AddComponent(new ComponentTransform((currentNode->worldPosition.x + (rightExtent / 2.0f) * nodeSize) - (nodeSize / 2.0f), 1.5f, currentNode->worldPosition.z));
						wall->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
						wall->GetTransformComponent()->SetScale(glm::vec3(rightExtent * (nodeSize / 2.0f), 1.5f, nodeSize / 2.0f));
						wall->GetGeometryComponent()->ApplyMaterialToModel(wallMaterial);
						wall->GetGeometryComponent()->SetTextureScale(glm::vec2(rightExtent, 1.0f));
						entityManager->AddEntity(wall);
						std::cout << "Wall " << count << " created" << std::endl;

						x += rightExtent;

						count++;
					}
					else {
						//Wall scales down
						Entity* wall = new Entity(std::string("Wall ") + std::string(std::to_string(count)));
						wall->AddComponent(new ComponentTransform(currentNode->worldPosition.x, 1.5f, (currentNode->worldPosition.z + (downExtent / 2.0f) * nodeSize) - nodeSize));
						wall->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
						wall->GetTransformComponent()->SetScale(glm::vec3(nodeSize / 2.0f, 1.5f, downExtent * (nodeSize / 2.0f)));
						wall->GetGeometryComponent()->ApplyMaterialToModel(wallMaterial);
						wall->GetGeometryComponent()->SetTextureScale(glm::vec2(downExtent, 1.0f));
						entityManager->AddEntity(wall);
						std::cout << "Wall " << count << " created" << std::endl;

						count++;
					}
				}
			}
		}

		glm::vec3 start = glm::vec3(8.0f, 0.0f, 1.0f) * nodeSize;

		Entity* target = new Entity("Target");
		target->AddComponent(new ComponentTransform(start.x, 1.0f, start.z + 12.5f));
		target->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		target->GetGeometryComponent()->ApplyMaterialToModel(path);
		target->GetTransformComponent()->SetScale(glm::vec3(0.5f)* (nodeSize * 0.5f));
		target->AddComponent(new ComponentLight(POINT));
		target->GetLightComponent()->Colour = glm::vec3(0.0f, 0.0f, 1.75f);
		target->GetLightComponent()->CastShadows = true;
		entityManager->AddEntity(target);

		Entity* agent = new Entity("Agent");
		agent->AddComponent(new ComponentTransform(start.x, 0.75f, start.z));
		agent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		agent->GetTransformComponent()->SetScale(glm::vec3(0.5f, 2.5f, 0.5f) * (nodeSize * 0.5f));
		agent->AddComponent(new ComponentPathfinder(navGrid, 10.0f * nodeSize, nodeSize));
		agent->GetGeometryComponent()->ApplyMaterialToModel(agentMaterial);
		agent->AddComponent(new ComponentLight(POINT));
		agent->GetLightComponent()->Colour = glm::vec3(1.75f);
		agent->GetLightComponent()->CastShadows = false;

		ComponentStateController* agentStateController = new ComponentStateController();

		// AI states
		StateFunc chase = [](void* data) {
			Entity* owner = (Entity*)data;

			if (owner) {
				bool success = owner->GetPathfinder()->FindPath(owner->GetTransformComponent()->GetWorldPosition(), owner->GetEntityManager()->FindEntity("Target")->GetTransformComponent()->GetWorldPosition());
			}
		};

		StateFunc chaseEnter = [](void* data) {
			Entity* owner = (Entity*)data;

			if (owner) {
				owner->GetPathfinder()->Reset();
				owner->GetPathfinder()->SetMoveSpeed(2.5f);
			}
		};

		StateFunc walkHomeEnter = [](void* data) {
			Entity* owner = (Entity*)data;
			if (owner) {
				owner->GetPathfinder()->Reset();
				owner->GetPathfinder()->SetMoveSpeed(1.5f);
				owner->GetPathfinder()->FindPath(owner->GetTransformComponent()->GetWorldPosition(), glm::vec3(2.0f, 1.0f, 0.25f));
			}
		};

		StateFunc patrolEnter = [](void* data) {
			Entity* owner = (Entity*)data;

			if (owner) {

				std::vector<glm::vec3> patrolPoints;
				patrolPoints.push_back(glm::vec3(1.0f, 1.0f, 0.25f));
				patrolPoints.push_back(glm::vec3(4.0f, 1.0f, 0.25f));
				patrolPoints.push_back(glm::vec3(1.0f, 1.0f, 3.25f));
				patrolPoints.push_back(glm::vec3(4.0f, 1.0f, 3.25f));

				int random = Random(0.0f, patrolPoints.size());

				glm::vec3 randomPatrolPoint = patrolPoints[random];

				glm::vec3 currentPos = owner->GetTransformComponent()->GetWorldPosition();

				while (glm::distance(randomPatrolPoint, currentPos) <= 0.3f) {
					// Already at this patrol point, find another
					patrolPoints.erase(patrolPoints.begin() + random);

					int random = Random(0.0f, patrolPoints.size());

					randomPatrolPoint = patrolPoints[random];
				}

				owner->GetPathfinder()->SetMoveSpeed(0.5f);
				owner->GetPathfinder()->FindPath(currentPos, randomPatrolPoint);
			}
		};

		// Idle state
		IdleState* idle = new IdleState();
		agentStateController->GetStateMachine().AddState(idle);

		// Chase state
		GenericState* chaseState = new GenericState("Chase", chase, nullptr, chaseEnter);
		agentStateController->GetStateMachine().AddState(chaseState);

		// Walk home state
		GenericState* walkHomeState = new GenericState("Walk Home", nullptr, nullptr, walkHomeEnter);
		agentStateController->GetStateMachine().AddState(walkHomeState);

		// Patrol state
		GenericState* patrolState = new GenericState("Patrol", nullptr, nullptr, patrolEnter);
		agentStateController->GetStateMachine().AddState(patrolState);

		using EntityBoolBoolBool = GenericStateTransition<Entity*, bool, bool, bool>;
		using EntityFloatFloatFloat = GenericStateTransition<Entity*, float, float, float>;
		using FloatFloatBoolBool = GenericStateTransition<float, float, bool, bool>;

		EntityBoolBoolBool::GenericDataRetrieverFuncA hasReachedDestination = [](EntityBoolBoolBool* owner) -> bool {
			Entity* agent = owner->GetSourceState()->GetParentStateMachine()->GetParentComponent()->GetOwner();
			if (agent) {
				return agent->GetPathfinder()->HasReachedTarget();
			}
			else {
				return true;
			}
		};

		EntityFloatFloatFloat::GenericDataRetrieverFuncA distanceToHome = [](EntityFloatFloatFloat* owner) -> float {
			Entity* agent = owner->GetSourceState()->GetParentStateMachine()->GetParentComponent()->GetOwner();
			if (agent) {
				return glm::distance(agent->GetTransformComponent()->GetWorldPosition(), glm::vec3(2.0f, 1.0f, 0.25f));
			}
			else {
				return 0.0f;
			}
		};

		EntityFloatFloatFloat::GenericDataRetrieverFuncA distanceToTarget = [](EntityFloatFloatFloat* owner) -> float {
			Entity* agent = owner->GetSourceState()->GetParentStateMachine()->GetParentComponent()->GetOwner();
			if (agent) {
				Entity* target = owner->GetBaseDataA();

				return glm::distance(agent->GetTransformComponent()->GetWorldPosition(), target->GetTransformComponent()->GetWorldPosition());
			}
			else {
				return 5.0f;
			}
		};

		FloatFloatBoolBool::GenericDataRetrieverFuncA updateSecondsWaited = [](FloatFloatBoolBool* owner) -> bool {
			float secondsWaited = owner->GetBaseDataA();
			float secondsToWait = owner->GetBaseDataB();

			secondsWaited += Scene::dt;

			bool readyToPatrol = false;
			if (secondsWaited >= secondsToWait) {
				readyToPatrol = true;
				secondsWaited = 0.0f;
				secondsToWait = Random(3.5f, 10.0f);
				owner->SetBaseDataB(secondsToWait);
			}

			owner->SetBaseDataA(secondsWaited);

			return readyToPatrol;
		};

		FloatFloatBoolBool::GenericDataRetrieverFuncB truePassthrough = [](FloatFloatBoolBool* owner) -> bool {
			return true;
		};

		EntityBoolBoolBool* patrolToIdleTransition = new EntityBoolBoolBool(EntityBoolBoolBool::EqualToCondition, nullptr, true, patrolState, idle, hasReachedDestination, EntityBoolBoolBool::PassthroughB);
		agentStateController->GetStateMachine().AddTransition(patrolToIdleTransition);

		EntityFloatFloatFloat* walkHomeToIdleTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::LessThanCondition, nullptr, 1.5f, walkHomeState, idle, distanceToHome, EntityFloatFloatFloat::PassthroughB);
		agentStateController->GetStateMachine().AddTransition(walkHomeToIdleTransition);

		EntityFloatFloatFloat* walkHomeToChaseTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::LessThanCondition, target, 5.0f, walkHomeState, chaseState, distanceToTarget, EntityFloatFloatFloat::PassthroughB);
		agentStateController->GetStateMachine().AddTransition(walkHomeToChaseTransition);

		EntityFloatFloatFloat* idleToChaseTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::LessThanCondition, target, 5.0f, idle, chaseState, distanceToTarget, EntityFloatFloatFloat::PassthroughB);
		agentStateController->GetStateMachine().AddTransition(idleToChaseTransition);

		FloatFloatBoolBool* idleToPatrolTransition = new FloatFloatBoolBool(FloatFloatBoolBool::EqualToCondition, 0.0f, 3.5f, idle, patrolState, updateSecondsWaited, truePassthrough);
		agentStateController->GetStateMachine().AddTransition(idleToPatrolTransition);

		EntityFloatFloatFloat* chaseToWalkHomeTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::GreaterThanCondition, target, 5.0f, chaseState, walkHomeState, distanceToTarget, EntityFloatFloatFloat::PassthroughB);
		agentStateController->GetStateMachine().AddTransition(chaseToWalkHomeTransition);

		EntityFloatFloatFloat* idleToWalkHomeTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::GreaterThanCondition, nullptr, 7.5f, idle, walkHomeState, distanceToHome, EntityFloatFloatFloat::PassthroughB);
		agentStateController->GetStateMachine().AddTransition(idleToWalkHomeTransition);

		agent->AddComponent(agentStateController);
		entityManager->AddEntity(agent);

		Entity* cloneTestBase = new Entity("Clone Test Base");
		cloneTestBase->AddComponent(new ComponentTransform(25.0f, 15.0f, 25.0f));
		cloneTestBase->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		cloneTestBase->GetGeometryComponent()->ApplyMaterialToModel(agentMaterial);
		cloneTestBase->GetTransformComponent()->SetScale(2.5f);
		entityManager->AddEntity(cloneTestBase);

		Entity* cloneTest1 = cloneTestBase->Clone();
		cloneTest1->GetTransformComponent()->SetPosition(cloneTest1->GetTransformComponent()->GetWorldPosition() + glm::vec3(15.0f, 0.0f, 0.0f));
		cloneTest1->GetGeometryComponent()->ApplyMaterialToModel(path);

		Entity* agentCloneTest = agent->Clone();
		agentCloneTest->GetTransformComponent()->SetPosition(glm::vec3(start.x, agentCloneTest->GetTransformComponent()->GetWorldPosition().y, start.z + 35.0f));

		Entity* agentCloneTest2 = agentCloneTest->Clone();
		agentCloneTest2->GetTransformComponent()->SetPosition(glm::vec3(start.x + 25.0f, agentCloneTest2->GetTransformComponent()->GetWorldPosition().y, start.z + 35.0f));

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

		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("AI State: "), glm::vec2(30.0f, SCR_HEIGHT - 80.0f), glm::vec2(0.35f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f), UIBackground(glm::vec4(0.03f, 0.4f, 0.1f, 0.05f), 0.035f, true, glm::vec4(1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("AI State: "), glm::vec2(30.0f, SCR_HEIGHT - 190.0f), glm::vec2(0.35f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f), UIBackground(glm::vec4(0.03f, 0.4f, 0.1f, 0.05f), 0.035f, true, glm::vec4(1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))));
		entityManager->AddEntity(canvas);
	}

	void AIScene::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemStateMachineUpdater(), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemPathfinding(), UPDATE_SYSTEMS);
		systemManager->AddCollisionResponseSystem(new CollisionResolver(collisionManager));
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
	}
}