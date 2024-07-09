#pragma once
#include "Scene.h"
#include "StateMachine.h"
#include "GenericState.h"
#include "GenericStateTransition.h"
#include "NavigationGrid.h"
namespace Engine {
	class AIScene : public Scene
	{
	public:
		AIScene(SceneManager* sceneManager);
		~AIScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

	private:
		StateMachine* stateMachine;
		GenericState* stateA;
		GenericState* stateB;
		GenericState* stateC;
		GenericStateTransition<int&, int>* transitionA;
		GenericStateTransition<int&, int>* transitionB;
		GenericStateTransition<int&, int>* transitionC;
		NavigationGrid* navGrid;
		NavigationPath navPath;
		int* someData;

		void CreateEntities();
		void CreateSystems();
	};
}