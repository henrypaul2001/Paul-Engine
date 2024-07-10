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

		camera->Position = glm::vec3(50.0f, 130.0f, 75.0f);

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
		navGrid = new NavigationGrid("Data/NavigationGrid/TestGrid1.txt");

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
		directional->Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		//directional->Colour = glm::vec3(1.0f, 1.0f, 1.5f) * 10.0f;
		directional->Colour = glm::vec3(0.7f, 0.65f, 0.85f);
		directional->Specular = glm::vec3(0.7f, 0.65f, 0.85f);
		directional->ShadowProjectionSize = 15.0f;
		directional->Far = 150.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(45.0f, 0.0f, 45.0f));
		floor->GetTransformComponent()->SetScale(glm::vec3(50.0f, 0.5f, 50.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		floor->GetAABBCollisionComponent()->IsMovedByCollisions(false);
		entityManager->AddEntity(floor);

		glm::vec3 start = glm::vec3(80.0f, 0.0f, 10.0f);
		glm::vec3 end = glm::vec3(80.0f, 0.0f, 80.0f);

		end = glm::vec3(40.0f, 0.0f, 80.0f);

		bool success = navGrid->FindPath(start, end, navPath);

		PBRMaterial* agentMaterial = new PBRMaterial();
		agentMaterial->albedo = glm::vec3(100.0f);

		Entity* agent = new Entity("Agent");
		agent->AddComponent(new ComponentTransform(start.x, 2.5f, start.z));
		agent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		agent->GetTransformComponent()->SetScale(1.0f, 4.0f, 1.0f);
		agent->AddComponent(new ComponentPathfinder(navGrid));
		agent->GetPathfinder()->FindPath(start, end);
		agent->GetGeometryComponent()->ApplyMaterialToModel(agentMaterial);
		entityManager->AddEntity(agent);

		PBRMaterial* walkable = new PBRMaterial();
		walkable->albedo = glm::vec3(0.0f, 100.0f, 0.0f);

		PBRMaterial* nonWalkable = new PBRMaterial();
		nonWalkable->albedo = glm::vec3(100.0f, 0.0f, 0.0f);

		PBRMaterial* path = new PBRMaterial();
		path->albedo = glm::vec3(0.0f, 0.0f, 100.0f);

		// Pathfinding debug grid
		Entity* baseInstanceNonWalkable = new Entity("Base Instance Non Walkable");
		baseInstanceNonWalkable->AddComponent(new ComponentTransform(0.0f, -10.0f, 0.0f));
		baseInstanceNonWalkable->AddComponent(new ComponentGeometry(MODEL_SPHERE, true, true));
		baseInstanceNonWalkable->GetGeometryComponent()->CastShadows(false);
		baseInstanceNonWalkable->GetGeometryComponent()->ApplyMaterialToModel(nonWalkable);
		entityManager->AddEntity(baseInstanceNonWalkable);

		int xNum = navGrid->GetGridWidth();
		int zNum = navGrid->GetGridHeight();

		float originX = 0.0f;
		float originZ = 0.0f;

		float nodeSize = 10.0f;

		std::vector<NavGridNode*> nodes = navGrid->GetNodes();

		std::vector<Entity*> gridEntities;
		gridEntities.reserve(xNum * zNum);

		int count = 0;
		for (int j = 0; j < xNum; j++) {
			for (int k = 0; k < zNum; k++) {


				if (nodes[count]->type == 'x') {
					std::string name = std::string("Box ") + std::string(std::to_string(count));
					Entity* box = new Entity(name);
					box->AddComponent(new ComponentTransform(nodes[count]->worldPosition.x, nodes[count]->worldPosition.y + 3.0f, nodes[count]->worldPosition.z));

					// Non walkable node
					baseInstanceNonWalkable->GetGeometryComponent()->AddNewInstanceSource(box);

					gridEntities.push_back(box);
					entityManager->AddEntity(box);
					std::cout << "box " << count << " created" << std::endl;
				}
				else {
					//std::string name = std::string("Box ") + std::string(std::to_string(count));
					//Entity* box = new Entity(name);
					//box->AddComponent(new ComponentTransform(nodes[count]->worldPosition.x, nodes[count]->worldPosition.y + 3.0f, nodes[count]->worldPosition.z));

					//box->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
					//box->GetGeometryComponent()->ApplyMaterialToModel(walkable);
					//box->GetGeometryComponent()->CastShadows(false);
				
					//gridEntities.push_back(box);
					//entityManager->AddEntity(box);
					//count++;
					//std::cout << "box " << count << " created" << std::endl;
				}

				count++;
			}
		}

		//for (const glm::vec3& waypoint : navPath.GetWaypointStack()._Get_container()) {
		//	// find start and end node indices
		//	int x = (waypoint.x / nodeSize);
		//	int y = (waypoint.z / nodeSize);

		//	Entity* gridEntity = gridEntities[(y * xNum) + x];
		//	gridEntity->GetGeometryComponent()->ApplyMaterialToModel(path);
		//}

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