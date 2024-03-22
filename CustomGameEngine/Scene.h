#pragma once
#include "ComponentTransform.h"
#include "ComponentVelocity.h"
#include "ComponentGeometry.h"
#include "ComponentLight.h"
#include "Camera.h"
#include "SystemManager.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "RenderManager.h"
#include "CollisionManager.h"
namespace Engine 
{
	class SceneManager;

	class Scene
	{
	protected:
		SceneManager* sceneManager;
		InputManager* inputManager;
		SystemManager* systemManager;
		EntityManager* entityManager;
		RenderManager* renderManager;
		CollisionManager* collisionManager;
		int SCR_WIDTH;
		int SCR_HEIGHT;

		Camera* camera;
	public:
		static float dt;
		Scene(SceneManager* sceneManager);
		~Scene();

		virtual void Render();
		virtual void Update();
		virtual void Close() = 0;
		virtual void SetupScene() = 0;

		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;

		InputManager* GetInputManager();
		SystemManager* GetSystemManager();
		Camera* GetCamera() { return camera; }
	};
}