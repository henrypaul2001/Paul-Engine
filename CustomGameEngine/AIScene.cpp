#include "AIScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "IdleState.h"
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

	void AIScene::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();

		float time = (float)glfwGetTime();
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
		
		// Player movement
		Entity* agent = ecs.Find("Agent");
		Entity* agentClone = ecs.Find("Agent (1)");
		Entity* target = ecs.Find("Target");

		const float moveSpeed = 2.5f * Scene::dt;
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(target->ID());
		if (inputManager->IsKeyDown(GLFW_KEY_KP_6)) {
			// Move right
			transform->SetPosition(transform->GetWorldPosition() + glm::vec3(moveSpeed, 0.0f, 0.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_4)) {
			// Move left
			transform->SetPosition(transform->GetWorldPosition() + glm::vec3(-moveSpeed, 0.0f, 0.0f));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_5)) {
			// Move down
			transform->SetPosition(transform->GetWorldPosition() + glm::vec3(0.0f, 0.0f, moveSpeed));
		}
		if (inputManager->IsKeyDown(GLFW_KEY_KP_8)) {
			// Move up
			transform->SetPosition(transform->GetWorldPosition() + glm::vec3(0.0f, 0.0f, -moveSpeed));
		}

		systemManager.ActionSystems();

		ComponentStateController* agentController = ecs.GetComponent<ComponentStateController>(agent->ID());
		ComponentStateController* agentCloneController = ecs.GetComponent<ComponentStateController>(agentClone->ID());

		dynamic_cast<UIText*>(canvas->UIElements()[10])->SetText("AI State: " + agentController->GetStateMachine().GetActiveStateName());
		dynamic_cast<UIText*>(canvas->UIElements()[11])->SetText("AI State: " + agentCloneController->GetStateMachine().GetActiveStateName());
	}

	void AIScene::Render()
	{
		Scene::Render();
	}

	void AIScene::Close() {}

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
			Entity* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void AIScene::keyDown(int key)
	{
		
	}

	void AIScene::CreateEntities()
	{
		Entity* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Colour = glm::vec3(0.7f, 0.65f, 0.85f) * 2.0f;
		directional.Specular = directional.Colour;
		directional.Ambient = directional.Colour * 200.0f;
		directional.Far = 300.0f;
		directional.DirectionalLightDistance = 150.0;
		directional.ShadowProjectionSize = 100.0f;
		directional.CastShadows = false;
		ecs.AddComponent(dirLight->ID(), directional);

		PBRMaterial* agentMaterial = new PBRMaterial(glm::vec3(100.0f));
		agentMaterial->roughness = 1.0f;
		agentMaterial->metallic = 0.0f;
		agentMaterial->ao = 0.0f;
		resources->AddMaterial("agentMaterial", agentMaterial);

		PBRMaterial* walkable = new PBRMaterial(glm::vec3(100.0f, 0.0f, 0.0f));
		walkable->roughness = 1.0f;
		walkable->metallic = 0.0f;
		walkable->ao = 1.0f;
		resources->AddMaterial("walkable", walkable);

		PBRMaterial* path = new PBRMaterial(glm::vec3(0.0f, 0.0f, 100.0f));
		path->roughness = 1.0f;
		path->metallic = 0.0f;
		path->ao = 0.0f;
		resources->AddMaterial("path", path);

		PBRMaterial* grass = new PBRMaterial(glm::vec3(65.0f, 152.0f, 10.0f) / 100.0f);
		grass->roughness = 1.0f;
		grass->metallic = 0.0f;
		grass->ao = 0.0f;
		resources->AddMaterial("grass", grass);

		PBRMaterial* wallMaterial = new PBRMaterial(glm::vec3(248.0f, 229.0f, 187.0f) / 100.0f);
		wallMaterial->roughness = 1.0f;
		wallMaterial->metallic = 0.0f;
		wallMaterial->ao = 0.0f;
		resources->AddMaterial("wallMaterial", wallMaterial);

		// Pathfinding debug grid
		//Entity* baseInstanceWalkable = ecs.New("Base Instance Non Walkable");
		//ecs.GetComponent<ComponentTransform>(baseInstanceWalkable->ID())->SetPosition(glm::vec3(0.0f, -10.0f, 0.0f));
		//ecs.AddComponent(baseInstanceWalkable->ID(), ComponentGeometry(MODEL_SPHERE, true, true));
		//ecs.GetComponent<ComponentGeometry>(baseInstanceWalkable->ID())->CastShadows(false);
		//ecs.GetComponent<ComponentGeometry>(baseInstanceWalkable->ID())->ApplyMaterialToModel(walkable);

		int xNum = navGrid->GetGridWidth();
		int zNum = navGrid->GetGridHeight();

		float originX = 0.0f;
		float originZ = 0.0f;

		float nodeSize = navGrid->GetNodeSize();

		const std::vector<NavGridNode*>& nodes = navGrid->GetNodes();

		/*
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
		*/

		// Construct scene based on navigation grid
		glm::vec3 floorScale = glm::vec3(((xNum - 1.0f) * nodeSize) / 2.0f, 0.5f, ((zNum - 1.0f) * nodeSize) / 2.0f);
		Entity* floor = ecs.New("Floor");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(((xNum - 1.0f) * nodeSize) / 2.0f, 0.0f, ((zNum - 1.0f) * nodeSize) / 2.0f));
		transform->SetScale(floorScale);
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.AddComponent(floor->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(floor->ID())->IsMovedByCollisions(false);
		ecs.GetComponent<ComponentGeometry>(floor->ID())->ApplyMaterialToModel(grass);

		camera->SetPosition(glm::vec3(transform->GetWorldPosition().x, ((xNum + zNum) * nodeSize) - 20.0f, transform->GetWorldPosition().z + nodeSize * 3));
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
						Entity* wall = ecs.New(std::string("Wall ") + std::string(std::to_string(count)));
						transform = ecs.GetComponent<ComponentTransform>(wall->ID());
						transform->SetPosition(glm::vec3(currentNode->worldPosition.x, 1.5f, currentNode->worldPosition.z));
						ecs.AddComponent(wall->ID(), ComponentGeometry(MODEL_CUBE, true));
						transform->SetScale(glm::vec3(nodeSize / 2.0f, 1.5f, nodeSize / 2.0f));
						ecs.GetComponent<ComponentGeometry>(wall->ID())->ApplyMaterialToModel(wallMaterial);
						std::cout << "Wall " << count << " created" << std::endl;

						count++;
					}
					else if (rightExtent > downExtent) {
						// Wall scales to right
						Entity* wall = ecs.New(std::string("Wall ") + std::string(std::to_string(count)));
						transform = ecs.GetComponent<ComponentTransform>(wall->ID());
						transform->SetPosition(glm::vec3((currentNode->worldPosition.x + (rightExtent / 2.0f) * nodeSize) - (nodeSize / 2.0f), 1.5f, currentNode->worldPosition.z));
						ecs.AddComponent(wall->ID(), ComponentGeometry(MODEL_CUBE, true));
						transform->SetScale(glm::vec3(rightExtent * (nodeSize / 2.0f), 1.5f, nodeSize / 2.0f));
						ecs.GetComponent<ComponentGeometry>(wall->ID())->ApplyMaterialToModel(wallMaterial);
						ecs.GetComponent<ComponentGeometry>(wall->ID())->SetTextureScale(glm::vec2(rightExtent, 1.0f));
						std::cout << "Wall " << count << " created" << std::endl;

						x += rightExtent;

						count++;
					}
					else {
						//Wall scales down
						Entity* wall = ecs.New(std::string("Wall ") + std::string(std::to_string(count)));
						transform = ecs.GetComponent<ComponentTransform>(wall->ID());
						transform->SetPosition(glm::vec3(currentNode->worldPosition.x, 1.5f, (currentNode->worldPosition.z + (downExtent / 2.0f) * nodeSize) - nodeSize));
						ecs.AddComponent(wall->ID(), ComponentGeometry(MODEL_CUBE, true));
						transform->SetScale(glm::vec3(nodeSize / 2.0f, 1.5f, downExtent * (nodeSize / 2.0f)));
						ecs.GetComponent<ComponentGeometry>(wall->ID())->ApplyMaterialToModel(wallMaterial);
						ecs.GetComponent<ComponentGeometry>(wall->ID())->SetTextureScale(glm::vec2(downExtent, 1.0f));
						std::cout << "Wall " << count << " created" << std::endl;

						count++;
					}
				}
			}
		}

		glm::vec3 start = glm::vec3(8.0f, 0.0f, 1.0f) * nodeSize;

		Entity* target = ecs.New("Target");
		transform = ecs.GetComponent<ComponentTransform>(target->ID());
		transform->SetPosition(glm::vec3(start.x, 1.0f, start.z + 12.5f));
		ecs.AddComponent(target->ID(), ComponentGeometry(MODEL_CUBE, true));
		ecs.GetComponent<ComponentGeometry>(target->ID())->ApplyMaterialToModel(path);
		transform->SetScale(glm::vec3(0.5f) * (nodeSize * 0.5f));
		ComponentLight light = ComponentLight(POINT);
		light.Colour = glm::vec3(0.0f, 0.0f, 1.75f);
		light.CastShadows = true;
		ecs.AddComponent(target->ID(), light);

		Entity* agent = ecs.New("Agent");
		transform = ecs.GetComponent<ComponentTransform>(agent->ID());
		transform->SetPosition(glm::vec3(start.x, 0.75f, start.z));
		ecs.AddComponent(agent->ID(), ComponentGeometry(MODEL_CUBE, true));
		transform->SetScale(glm::vec3(0.5f, 2.5f, 0.5f) * (nodeSize * 0.5f));
		ecs.AddComponent(agent->ID(), ComponentPathfinder(navGrid, 10.0f * nodeSize, nodeSize));
		ecs.GetComponent<ComponentGeometry>(agent->ID())->ApplyMaterialToModel(agentMaterial);
		light = ComponentLight(POINT);
		light.Colour = glm::vec3(1.75f);
		light.CastShadows = false;
		ecs.AddComponent(agent->ID(), light);

		ComponentStateController agentStateController = ComponentStateController();

		// AI states
		StateFunc chase = [](void* data, EntityManager* ecs, const unsigned int entityID) {
			ComponentTransform* transform = ecs->GetComponent<ComponentTransform>(entityID);
			ComponentPathfinder* pathfinder = ecs->GetComponent<ComponentPathfinder>(entityID);
			bool success = pathfinder->FindPath(transform->GetWorldPosition(), ecs->GetComponent<ComponentTransform>(ecs->Find("Target")->ID())->GetWorldPosition());
		};

		StateFunc chaseEnter = [](void* data, EntityManager* ecs, const unsigned int entityID) {
			ComponentPathfinder* pathfinder = ecs->GetComponent<ComponentPathfinder>(entityID);
			pathfinder->Reset();
			pathfinder->SetMoveSpeed(2.5f);
		};

		StateFunc walkHomeEnter = [](void* data, EntityManager* ecs, const unsigned int entityID) {
			ComponentTransform* transform = ecs->GetComponent<ComponentTransform>(entityID);
			ComponentPathfinder* pathfinder = ecs->GetComponent<ComponentPathfinder>(entityID);
			pathfinder->Reset();
			pathfinder->SetMoveSpeed(1.5f);
			pathfinder->FindPath(transform->GetWorldPosition(), glm::vec3(2.0f, 1.0f, 0.25f));
		};

		StateFunc patrolEnter = [](void* data, EntityManager* ecs, const unsigned int entityID) {
			ComponentTransform* transform = ecs->GetComponent<ComponentTransform>(entityID);
			ComponentPathfinder* pathfinder = ecs->GetComponent<ComponentPathfinder>(entityID);

			std::vector<glm::vec3> patrolPoints;
			patrolPoints.push_back(glm::vec3(1.0f, 1.0f, 0.25f));
			patrolPoints.push_back(glm::vec3(4.0f, 1.0f, 0.25f));
			patrolPoints.push_back(glm::vec3(1.0f, 1.0f, 3.25f));
			patrolPoints.push_back(glm::vec3(4.0f, 1.0f, 3.25f));

			int random = Random(0.0f, patrolPoints.size());

			glm::vec3 randomPatrolPoint = patrolPoints[random];

			glm::vec3 currentPos = transform->GetWorldPosition();

			while (glm::distance(randomPatrolPoint, currentPos) <= 0.3f) {
				// Already at this patrol point, find another
				patrolPoints.erase(patrolPoints.begin() + random);

				int random = Random(0.0f, patrolPoints.size());

				randomPatrolPoint = patrolPoints[random];
			}

			pathfinder->SetMoveSpeed(0.5f);
			pathfinder->FindPath(currentPos, randomPatrolPoint);
		};

		// Idle state
		IdleState* idle = new IdleState();
		agentStateController.GetStateMachine().AddState(idle);

		// Chase state
		GenericState* chaseState = new GenericState("Chase", chase, nullptr, chaseEnter);
		agentStateController.GetStateMachine().AddState(chaseState);

		// Walk home state
		GenericState* walkHomeState = new GenericState("Walk Home", nullptr, nullptr, walkHomeEnter);
		agentStateController.GetStateMachine().AddState(walkHomeState);

		// Patrol state
		GenericState* patrolState = new GenericState("Patrol", nullptr, nullptr, patrolEnter);
		agentStateController.GetStateMachine().AddState(patrolState);

		using EntityBoolBoolBool = GenericStateTransition<bool, bool, bool, bool>;
		using EntityFloatFloatFloat = GenericStateTransition<std::string, float, float, float>;
		using FloatFloatBoolBool = GenericStateTransition<float, float, bool, bool>;

		EntityBoolBoolBool::GenericDataRetrieverFuncA hasReachedDestination = [](EntityBoolBoolBool* owner, EntityManager* ecs, const unsigned int entityID) -> bool {
			return ecs->GetComponent<ComponentPathfinder>(entityID)->HasReachedTarget();
		};

		EntityFloatFloatFloat::GenericDataRetrieverFuncA distanceToHome = [](EntityFloatFloatFloat* owner, EntityManager* ecs, const unsigned int entityID) -> float {
			return glm::distance(ecs->GetComponent<ComponentTransform>(entityID)->GetWorldPosition(), glm::vec3(2.0f, 1.0f, 0.25f));
		};

		EntityFloatFloatFloat::GenericDataRetrieverFuncA distanceToTarget = [](EntityFloatFloatFloat* owner, EntityManager* ecs, const unsigned int entityID) -> float {
			const std::string& targetName = owner->GetBaseDataA();

			return glm::distance(ecs->GetComponent<ComponentTransform>(entityID)->GetWorldPosition(), ecs->GetComponent<ComponentTransform>(ecs->Find(targetName)->ID())->GetWorldPosition());
		};

		FloatFloatBoolBool::GenericDataRetrieverFuncA updateSecondsWaited = [](FloatFloatBoolBool* owner, EntityManager* ecs, const unsigned int entityID) -> bool {
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

		FloatFloatBoolBool::GenericDataRetrieverFuncB truePassthrough = [](FloatFloatBoolBool* owner, EntityManager* ecs, const unsigned int entityID) -> bool {
			return true;
		};

		EntityBoolBoolBool* patrolToIdleTransition = new EntityBoolBoolBool(EntityBoolBoolBool::EqualToCondition, false, true, patrolState, idle, hasReachedDestination, EntityBoolBoolBool::PassthroughB);
		agentStateController.GetStateMachine().AddTransition(patrolToIdleTransition);

		EntityFloatFloatFloat* walkHomeToIdleTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::LessThanCondition, "nullptr", 1.5f, walkHomeState, idle, distanceToHome, EntityFloatFloatFloat::PassthroughB);
		agentStateController.GetStateMachine().AddTransition(walkHomeToIdleTransition);

		EntityFloatFloatFloat* walkHomeToChaseTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::LessThanCondition, "Target", 5.0f, walkHomeState, chaseState, distanceToTarget, EntityFloatFloatFloat::PassthroughB);
		agentStateController.GetStateMachine().AddTransition(walkHomeToChaseTransition);

		EntityFloatFloatFloat* idleToChaseTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::LessThanCondition, "Target", 5.0f, idle, chaseState, distanceToTarget, EntityFloatFloatFloat::PassthroughB);
		agentStateController.GetStateMachine().AddTransition(idleToChaseTransition);

		FloatFloatBoolBool* idleToPatrolTransition = new FloatFloatBoolBool(FloatFloatBoolBool::EqualToCondition, 0.0f, 3.5f, idle, patrolState, updateSecondsWaited, truePassthrough);
		agentStateController.GetStateMachine().AddTransition(idleToPatrolTransition);

		EntityFloatFloatFloat* chaseToWalkHomeTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::GreaterThanCondition, "Target", 5.0f, chaseState, walkHomeState, distanceToTarget, EntityFloatFloatFloat::PassthroughB);
		agentStateController.GetStateMachine().AddTransition(chaseToWalkHomeTransition);

		EntityFloatFloatFloat* idleToWalkHomeTransition = new EntityFloatFloatFloat(EntityFloatFloatFloat::GreaterThanCondition, "nullptr", 7.5f, idle, walkHomeState, distanceToHome, EntityFloatFloatFloat::PassthroughB);
		agentStateController.GetStateMachine().AddTransition(idleToWalkHomeTransition);

		ecs.AddComponent(agent->ID(), agentStateController);
		unsigned int agentID = agent->ID();

		Entity* cloneTestBase = ecs.New("Clone Test Base");
		transform = ecs.GetComponent<ComponentTransform>(cloneTestBase->ID());
		transform->SetPosition(glm::vec3(25.0f, 15.0f, 25.0f));
		ecs.AddComponent(cloneTestBase->ID(), ComponentGeometry(MODEL_CUBE, true));
		ecs.GetComponent<ComponentGeometry>(cloneTestBase->ID())->ApplyMaterialToModel(agentMaterial);
		transform->SetScale(2.5f);

		unsigned int cloneTestBaseID = cloneTestBase->ID();

		Entity* cloneTest1 = ecs.Clone(cloneTestBaseID);
		transform = ecs.GetComponent<ComponentTransform>(cloneTest1->ID());
		transform->SetPosition(transform->GetWorldPosition() + glm::vec3(15.0f, 0.0f, 0.0f));
		ecs.GetComponent<ComponentGeometry>(cloneTest1->ID())->ApplyMaterialToModel(path);

		Entity* agentCloneTest = ecs.Clone(agentID);
		transform = ecs.GetComponent<ComponentTransform>(agentCloneTest->ID());
		transform->SetPosition(glm::vec3(start.x, transform->GetWorldPosition().y, start.z + 35.0f));

		Entity* agentCloneTest2 = ecs.Clone(agentCloneTest->ID());
		transform = ecs.GetComponent<ComponentTransform>(agentCloneTest2->ID());
		transform->SetPosition(glm::vec3(start.x + 25.0f, transform->GetWorldPosition().y, start.z + 35.0f));

#pragma region UI
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
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

		canvas->AddUIElement(new UIText(std::string("AI State: "), glm::vec2(30.0f, SCR_HEIGHT - 80.0f), glm::vec2(0.35f), resources->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f), UIBackground(glm::vec4(0.03f, 0.4f, 0.1f, 0.05f), 0.035f, true, glm::vec4(1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))));
		canvas->AddUIElement(new UIText(std::string("AI State: "), glm::vec2(30.0f, SCR_HEIGHT - 190.0f), glm::vec2(0.35f), resources->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f), UIBackground(glm::vec4(0.03f, 0.4f, 0.1f, 0.05f), 0.035f, true, glm::vec4(1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))));
#pragma endregion
	}

	void AIScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}