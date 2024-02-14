#pragma once
#include "ComponentTransform.h"
#include "ComponentVelocity.h"
#include "ComponentGeometry.h"
#include "Camera.h"
namespace Engine 
{
	class SceneManager;
	class InputManager;
	class SystemManager;
	class EntityManager;

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
		Camera camera;
	};
}