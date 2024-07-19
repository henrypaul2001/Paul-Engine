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

		float* distanceSqrClone;
		float* distanceToHomeSqrClone;
		bool* hasReachedDestinationClone;
		bool* readyToPatrolClone;

		float secondsToWait;
		float secondsWaited;

		float secondsToWaitClone;
		float secondsWaitedClone;

		void CreateEntities();
		void CreateSystems();

		static float Random(float min, float max) {
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
		};
	};
}