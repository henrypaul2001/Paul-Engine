#include "AIScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
#include "SystemPathfinding.h"
#include "IdleState.h"
#include "SystemStateMachineUpdater.h"
namespace Engine {
	AIScene::AIScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		distanceSqr = new float;
		distanceToHomeSqr = new float;
		hasReachedDestination = new bool;
		readyToPatrol = new bool;
		secondsToWait = Random(3.5f, 10.0f);
		SetupScene();
	}

	AIScene::~AIScene()
	{
		delete navGrid;
		delete distanceSqr;
		delete distanceToHomeSqr;
		delete hasReachedDestination;
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

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));
		
		// Player movement
		Entity * agent = entityManager->FindEntity("Agent");
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

		*distanceSqr = glm::distance2(target->GetTransformComponent()->GetWorldPosition(), agent->GetTransformComponent()->GetWorldPosition());
		*distanceToHomeSqr = glm::distance2(agent->GetTransformComponent()->GetWorldPosition(), glm::vec3(2.0f, 1.0f, 0.25f));
		*hasReachedDestination = agent->GetPathfinder()->HasReachedTarget();

		*readyToPatrol = false;
		std::string stateName = agent->GetStateController()->GetStateMachine().GetActiveStateName();

		if (std::string(stateName.begin(), stateName.begin() + 4) == "Idle") {
			secondsWaited += Scene::dt;

			if (secondsWaited >= secondsToWait) {
				*readyToPatrol = true;
				secondsWaited = 0.0f;
				secondsToWait = Random(3.5f, 10.0f);
			}
		}
		else {
			secondsWaited = 0.0f;
		}

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[2])->SetText("AI State: " + stateName);
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

		camera->Position = glm::vec3(floor->GetTransformComponent()->GetWorldPosition().x, (xNum + zNum) * nodeSize, floor->GetTransformComponent()->GetWorldPosition().z + nodeSize * 3);

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
		target->GetLightComponent()->Colour = glm::vec3(0.0f, 0.0f, 5.0f);
		target->GetLightComponent()->CastShadows = true;
		entityManager->AddEntity(target);

		Entity* agent = new Entity("Agent");
		agent->AddComponent(new ComponentTransform(start.x, 1.0f, start.z));
		agent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		agent->GetTransformComponent()->SetScale(glm::vec3(0.5f, 2.5f, 0.5f) * (nodeSize * 0.5f));
		agent->AddComponent(new ComponentPathfinder(navGrid, 10.0f * nodeSize, nodeSize));
		agent->GetGeometryComponent()->ApplyMaterialToModel(agentMaterial);
		agent->AddComponent(new ComponentLight(POINT));
		agent->GetLightComponent()->Colour = glm::vec3(5.0f);
		agent->GetLightComponent()->CastShadows = true;

		ComponentStateController* agentStateController = new ComponentStateController();

		// AI states
		StateFunc chase = [](void* data) {
			Entity* owner = (Entity*)data;

			if (owner) {
				if (owner->GetPathfinder()->HasReachedTarget()) {
					bool success = owner->GetPathfinder()->FindPath(owner->GetTransformComponent()->GetWorldPosition(), owner->GetEntityManager()->FindEntity("Target")->GetTransformComponent()->GetWorldPosition());
				}
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
		IdleState* idle = new IdleState(agent);
		agentStateController->GetStateMachine().AddState(idle);

		// Chase state
		GenericState* chaseState = new GenericState("Chase", chase, agent, chaseEnter);
		agentStateController->GetStateMachine().AddState(chaseState);

		// Walk home state
		GenericState* walkHomeState = new GenericState("Walk Home", nullptr, agent, walkHomeEnter);
		agentStateController->GetStateMachine().AddState(walkHomeState);

		// Patrol state
		GenericState* patrolState = new GenericState("Patrol", nullptr, agent, patrolEnter);
		agentStateController->GetStateMachine().AddState(patrolState);

		GenericStateTransition<bool&, bool>* patrolToIdleTransition = new GenericStateTransition<bool&, bool>(GenericStateTransition<bool&, bool>::EqualToCondition, *hasReachedDestination, true, patrolState, idle);
		agentStateController->GetStateMachine().AddTransition(patrolToIdleTransition);

		GenericStateTransition<float&, float>* patrolToChaseTransition = new GenericStateTransition<float&, float>(GenericStateTransition<float&, float>::LessThanCondition, *distanceSqr, (5.0f * 5.0f), patrolState, chaseState);
		agentStateController->GetStateMachine().AddTransition(patrolToChaseTransition);

		GenericStateTransition<float&, float>* walkHomeToIdleTransition = new GenericStateTransition<float&, float>(GenericStateTransition<float&, float>::LessThanCondition, *distanceToHomeSqr, (1.5f * 1.5f), walkHomeState, idle);
		agentStateController->GetStateMachine().AddTransition(walkHomeToIdleTransition);

		GenericStateTransition<float&, float>* walkHomeToChaseTransition = new GenericStateTransition<float&, float>(GenericStateTransition<float&, float>::LessThanCondition, *distanceSqr, (5.0f * 5.0f), walkHomeState, chaseState);
		agentStateController->GetStateMachine().AddTransition(walkHomeToChaseTransition);

		GenericStateTransition<float&, float>* idleToChaseTransition = new GenericStateTransition<float&, float>(GenericStateTransition<float&, float>::LessThanCondition, *distanceSqr, (5.0f * 5.0f), idle, chaseState);
		agentStateController->GetStateMachine().AddTransition(idleToChaseTransition);

		GenericStateTransition<bool&, bool>* idleToPatrolTransition = new GenericStateTransition<bool&, bool>(GenericStateTransition<bool&, bool>::EqualToCondition, *readyToPatrol, true, idle, patrolState);
		agentStateController->GetStateMachine().AddTransition(idleToPatrolTransition);

		GenericStateTransition<float&, float>* chaseToWalkHomeTransition = new GenericStateTransition<float&, float>(GenericStateTransition<float&, float>::GreaterThanCondition, *distanceSqr, (5.0f * 5.0f), chaseState, walkHomeState);
		agentStateController->GetStateMachine().AddTransition(chaseToWalkHomeTransition);

		agent->AddComponent(agentStateController);
		entityManager->AddEntity(agent);

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(30.0f, 80.0f), glm::vec2(0.25f, 0.25f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(30.0f, 30.0f), glm::vec2(0.20f, 0.20f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("AI State: "), glm::vec2(30.0f, SCR_HEIGHT - 80.0f), glm::vec2(0.35f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f), UIBackground(glm::vec4(0.03f, 0.4f, 0.1f, 0.05f), 0.035f, true, glm::vec4(1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))));
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
	}
}