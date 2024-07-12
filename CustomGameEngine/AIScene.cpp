#include "AIScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
#include "SystemPathfinding.h"
namespace Engine {
	AIScene::AIScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
	}

	AIScene::~AIScene()
	{
		delete stateMachine;
		delete stateA;
		delete stateB;
		delete stateC;
		delete transitionA;
		delete transitionB;
		delete transitionC;
		delete someData;
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

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		//stateMachine->Update();

		Entity* agent = entityManager->FindEntity("Agent");

		int iterations = 0;
		if (agent->GetPathfinder()->HasReachedTarget()) {
			bool success = false;
			while (!success) {
				// Find new target
				targetIndex++;
				if (targetIndex >= pathTargets.size()) {
					targetIndex = 0;
				}

				glm::vec3 target = pathTargets[targetIndex];
				success = agent->GetPathfinder()->FindPath(agent->GetTransformComponent()->GetWorldPosition(), target);

				Entity* targetEntity = entityManager->FindEntity("Target");
				targetEntity->GetTransformComponent()->SetPosition(glm::vec3(target.x, targetEntity->GetTransformComponent()->GetWorldPosition().y, target.z));
				iterations++;
				if (iterations > pathTargets.size() + 1) {
					break;
				}
			}
		}
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

		// State machine
		stateMachine = new StateMachine();

		someData = new int(0);

		StateFunc AFunc = [](void* data) {
			int* realData = (int*)data;
			(*realData)++;
			std::cout << *realData << " | ";
			std::cout << "In State A" << std::endl;
			};

		StateFunc BFunc = [](void* data) {
			int* realData = (int*)data;
			(*realData)--;
			std::cout << *realData << " | ";
			std::cout << "In State B" << std::endl;
			};

		StateFunc CFunc = [](void* data) {
			int* realData = (int*)data;
			(*realData) -= 4;
			std::cout << *realData << " | ";
			std::cout << "In State C" << std::endl;
			};

		stateA = new GenericState("State A", AFunc, (void*)someData);
		stateB = new GenericState("State B", BFunc, (void*)someData);
		stateC = new GenericState("State C", CFunc, (void*)someData);
		stateMachine->AddState(stateA);
		stateMachine->AddState(stateB);
		stateMachine->AddState(stateC);

		transitionA = new GenericStateTransition<int&, int>(GenericStateTransition<int&, int>::GreaterThanCondition, *someData, 100, stateA, stateB); // if "someData" is greater than 10, transition from state A to state B
		transitionB = new GenericStateTransition<int&, int>(GenericStateTransition<int&, int>::EqualToCondition, *someData, 0, stateB, stateC); // if "someData" is equal to 0, transition from state B to state C
		transitionC = new GenericStateTransition<int&, int>(GenericStateTransition<int&, int>::LessThanCondition, *someData, -100, stateC, stateA); // if "someData" is less than -100, transition from state C to state A

		stateMachine->AddTransition(transitionA);
		stateMachine->AddTransition(transitionB);
		stateMachine->AddTransition(transitionC);

		// Nav grid
		navGrid = new NavigationGrid("Data/NavigationGrid/TestGrid6.txt");

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

		glm::vec3 start = glm::vec3(80.0f, 0.0f, 10.0f);
		glm::vec3 end = glm::vec3(80.0f, 0.0f, 80.0f);

		targetIndex = 0;
		pathTargets.push_back(end);
		pathTargets.push_back(start);
		pathTargets.push_back(glm::vec3(60.0f, 0.0f, 10.0f));
		pathTargets.push_back(glm::vec3(40.0f, 0.0f, 50.0f));
		pathTargets.push_back(glm::vec3(20.0f, 0.0f, 20.0f));
		pathTargets.push_back(glm::vec3(10.0f, 0.0f, 80.0f));
		pathTargets.push_back(glm::vec3(60.0f, 0.0f, 20.0f));

		end = glm::vec3(40.0f, 0.0f, 80.0f);

		bool success = navGrid->FindPath(start, end, navPath);

		PBRMaterial* agentMaterial = new PBRMaterial();
		agentMaterial->albedo = glm::vec3(100.0f);

		PBRMaterial* walkable = new PBRMaterial();
		walkable->albedo = glm::vec3(0.0f, 100.0f, 0.0f);

		PBRMaterial* nonWalkable = new PBRMaterial();
		nonWalkable->albedo = glm::vec3(100.0f, 0.0f, 0.0f);

		PBRMaterial* path = new PBRMaterial();
		path->albedo = glm::vec3(0.0f, 0.0f, 100.0f);

		PBRMaterial* grass = new PBRMaterial();
		grass->albedo = glm::vec3(65.0f, 152.0f, 10.0f) / 100.0f;

		PBRMaterial* wallMaterial = new PBRMaterial();
		wallMaterial->albedo = glm::vec3(248.0f, 229.0f, 187.0f) / 100.0f;

		Entity* agent = new Entity("Agent");
		agent->AddComponent(new ComponentTransform(start.x, 3.0f, start.z));
		agent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		agent->GetTransformComponent()->SetScale(1.0f, 4.0f, 1.0f);
		agent->AddComponent(new ComponentPathfinder(navGrid, 17.5f, 0.15f));
		agent->GetPathfinder()->FindPath(start, end);
		agent->GetGeometryComponent()->ApplyMaterialToModel(agentMaterial);
		agent->AddComponent(new ComponentLight(POINT));
		agent->GetLightComponent()->Colour = glm::vec3(5.0f);
		agent->GetLightComponent()->CastShadows = false;
		entityManager->AddEntity(agent);

		Entity* target = new Entity("Target");
		target->AddComponent(new ComponentTransform(end.x, 3.0f, end.z));
		target->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		target->GetGeometryComponent()->ApplyMaterialToModel(path);
		target->AddComponent(new ComponentLight(POINT));
		target->GetLightComponent()->Colour = glm::vec3(0.0f, 0.0f, 5.0f);
		target->GetLightComponent()->CastShadows = false;
		entityManager->AddEntity(target);

		// Pathfinding debug grid
		Entity* baseInstanceNonWalkable = new Entity("Base Instance Non Walkable");
		baseInstanceNonWalkable->AddComponent(new ComponentTransform(0.0f, -10.0f, 0.0f));
		baseInstanceNonWalkable->AddComponent(new ComponentGeometry(MODEL_SPHERE, true, true));
		baseInstanceNonWalkable->GetGeometryComponent()->CastShadows(true);
		baseInstanceNonWalkable->GetGeometryComponent()->ApplyMaterialToModel(nonWalkable);
		entityManager->AddEntity(baseInstanceNonWalkable);

		int xNum = navGrid->GetGridWidth();
		int zNum = navGrid->GetGridHeight();

		float originX = 0.0f;
		float originZ = 0.0f;

		float nodeSize = navGrid->GetNodeSize();

		std::vector<NavGridNode*> nodes = navGrid->GetNodes();

		std::vector<Entity*> gridEntities;
		gridEntities.reserve(xNum * zNum);

		int count = 0;
		for (int j = 0; j < xNum; j++) {
			for (int k = 0; k < zNum; k++) {
				if (nodes[count]->type == 'x') {
					std::string name = std::string("Sphere ") + std::string(std::to_string(count));
					Entity* sphere = new Entity(name);
					sphere->AddComponent(new ComponentTransform(nodes[count]->worldPosition.x, nodes[count]->worldPosition.y + 3.0f, nodes[count]->worldPosition.z));

					// Non walkable node
					baseInstanceNonWalkable->GetGeometryComponent()->AddNewInstanceSource(sphere);

					gridEntities.push_back(sphere);
					entityManager->AddEntity(sphere);
				}
				count++;
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

		camera->Position = glm::vec3(floor->GetTransformComponent()->GetWorldPosition().x, 130.0f, floor->GetTransformComponent()->GetWorldPosition().z + nodeSize * 3);

		// Create walls
		count = 0;
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
						wall->GetTransformComponent()->SetScale(glm::vec3(nodeSize / 2.0f, 3.0f, nodeSize / 2.0f));
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
						wall->GetTransformComponent()->SetScale(glm::vec3(rightExtent * (nodeSize / 2.0f), 3.0f, nodeSize / 2.0f));
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
						wall->GetTransformComponent()->SetScale(glm::vec3(nodeSize / 2.0f, 3.0f, downExtent * (nodeSize / 2.0f)));
						wall->GetGeometryComponent()->ApplyMaterialToModel(wallMaterial);
						wall->GetGeometryComponent()->SetTextureScale(glm::vec2(downExtent, 1.0f));
						entityManager->AddEntity(wall);
						std::cout << "Wall " << count << " created" << std::endl;

						count++;
					}
				}
			}
		}

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(30.0f, 80.0f), glm::vec2(0.25f, 0.25f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(30.0f, 30.0f), glm::vec2(0.20f, 0.20f), ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf"), glm::vec3(0.0f, 0.0f, 0.0f)));
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
		systemManager->AddSystem(new SystemPathfinding(), UPDATE_SYSTEMS);
	}
}