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
#include "ConstraintManager.h"

#include "SystemFrustumCulling.h"
#include "UIText.h"
#include "SystemUIRender.h"
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
		ConstraintManager* constraintManager;
		ResourceManager* resources;
		int SCR_WIDTH;
		int SCR_HEIGHT;

		std::string name;

		Camera* camera;
	public:
		static float dt;
		Scene(SceneManager* sceneManager, const std::string& name);
		~Scene();

		virtual void OnSceneCreated();

		virtual void Render();
		virtual void Update();
		virtual void Close() = 0;
		virtual void SetupScene() = 0;

		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;

		const std::string& GetName() { return name; }

		InputManager* GetInputManager() const;
		SystemManager* GetSystemManager() const;
		SceneManager* GetSceneManager() const;
		Camera* GetCamera() { return camera; }
	};
}