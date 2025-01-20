#include "EmptyScene.h"
#include "GameInputManager.h"

namespace Engine {
	Engine::EmptyScene::EmptyScene(SceneManager* sceneManager) : Scene(sceneManager, "EmptyScene"), systemManager(&ecs), audioSystem(&ecs), physicsSystem(&ecs)
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);

		SetupScene();
	}
}