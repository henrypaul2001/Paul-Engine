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
	public:
		static float dt;
		Scene(SceneManager* sceneManager);
		~Scene();

		virtual void Render() = 0;
		virtual void Update() = 0;
		virtual void Close() = 0;
		virtual void SetupScene() = 0;

		InputManager* GetInputManager();
		SystemManager* GetSystemManager();
		Camera camera;
	};
}