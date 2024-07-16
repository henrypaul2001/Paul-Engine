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
		bool* hasReachedDestination;
		bool* readyToPatrol;

		float secondsToWait;
		float secondsWaited;

		void CreateEntities();
		void CreateSystems();

		static float Random(float min, float max) {
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
		};
	};
}