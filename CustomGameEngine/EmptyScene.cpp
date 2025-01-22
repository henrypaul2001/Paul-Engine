#include "EmptyScene.h"
#include "GameInputManager.h"

namespace Engine {
	Engine::EmptyScene::EmptyScene(SceneManager* sceneManager) : Scene(sceneManager, "EmptyScene"), systemManager(&ecs), audioSystem(&ecs), physicsSystem(&ecs), pathfindingSystem(&ecs), particleUpdater(&ecs), uiInteract(nullptr, nullptr), stateUpdater(&ecs), animSystem(&ecs), meshListSystem(&ecs), animAABBSystem(&ecs),
		aabbSystem(&ecs, collisionManager), boxSystem(&ecs, collisionManager), boxAABBSystem(&ecs, collisionManager), sphereSystem(&ecs, collisionManager), sphereAABBSystem(&ecs, collisionManager)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);

		uiInteract = SystemUIMouseInteraction(&ecs, inputManager);

		SetupScene();
	}
}