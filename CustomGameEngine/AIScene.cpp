#include "AIScene.h"
#include "GameInputManager.h"
#include "UIText.h"
#include "SystemUIRender.h"
namespace Engine {
	AIScene::AIScene(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();

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
		NavigationPath path;
		bool success = navGrid->FindPath(glm::vec3(80.0f, 0.0f, 10.0f), glm::vec3(80.0f, 0.0f, 80.0f), path);
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

		stateMachine->Update();
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
		floor->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		floor->GetTransformComponent()->SetScale(glm::vec3(10.0f, 0.5f, 10.0f));
		floor->AddComponent(new ComponentGeometry(MODEL_CUBE));
		floor->AddComponent(new ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		floor->GetAABBCollisionComponent()->IsMovedByCollisions(false);
		entityManager->AddEntity(floor);

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
	}
}