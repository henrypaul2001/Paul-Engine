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
		NavigationGrid* navGrid;
		NavigationPath navPath;

		float* distanceSqr;
		float* distanceToHomeSqr;

		void CreateEntities();
		void CreateSystems();
	};
}