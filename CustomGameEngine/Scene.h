#pragma once
#include "Camera.h"

#include "InputManager.h"
#include "ResourceManager.h"
#include "RenderManager.h"
#include "CollisionManager.h"
#include "ConstraintManager.h"
#include "LightManager.h"
#include "EntityManagerNew.h"

#include "ComponentPhysics.h"
#include "ComponentAnimator.h"
#include "ComponentAudioSource.h"
#include "ComponentCollisionSphere.h"
#include "ComponentCollisionBox.h"
#include "ComponentCollisionAABB.h"
#include "ComponentGeometry.h"
#include "ComponentLight.h"
#include "ComponentParticleGenerator.h"
#include "ComponentPathfinder.h"
#include "ComponentStateController.h"
#include "ComponentUICanvas.h"

#include "SystemManagerNew.h"
#include "SystemAudio.h"
#include "SystemPhysics.h"
#include "SystemPathfinding.h"
#include "SystemParticleUpdater.h"
#include "SystemUIMouseInteraction.h"
#include "SystemStateMachineUpdater.h"
#include "SystemSkeletalAnimationUpdater.h"
#include "SystemBuildMeshList.h"
#include "SystemAnimatedGeometryAABBGeneration.h"
#include "SystemLighting.h"
#include "SystemReflectionBaking.h"

#include "SystemCollisionAABB.h"
#include "SystemCollisionBox.h"
#include "SystemCollisionBoxAABB.h"
#include "SystemCollisionSphere.h"
#include "SystemCollisionSphereAABB.h"
#include "SystemCollisionsphereBox.h"

#include "CollisionResolver.h"
#include "ConstraintSolver.h"

namespace Engine 
{
	class SceneManager;

	class Scene
	{
	public:
		static float dt;
		Scene(SceneManager* sceneManager, const std::string& name);
		~Scene() {
			ResourceManager::GetInstance()->ClearTempResources();
			renderManager->GetBakedData().ClearBakedData();
			delete camera;
			delete constraintManager;
		}

		virtual void OnSceneCreated();

		virtual void Render();
		virtual void Update();
		virtual void Close() = 0;
		virtual void SetupScene() = 0;

		virtual void keyUp(int key) = 0;
		virtual void keyDown(int key) = 0;

		const std::string& GetName() const { return name; }

		InputManager* GetInputManager() const;
		SceneManager* GetSceneManager() const;
		Camera* GetCamera() { return camera; }
		const Camera* GetCamera() const { return camera; }

	protected:
		SceneManager* sceneManager;
		InputManager* inputManager;
		RenderManager* renderManager;
		CollisionManager* collisionManager;
		ConstraintManager* constraintManager;
		ResourceManager* resources;

		EntityManagerNew ecs;
		LightManager lightManager;
		SystemFrustumCulling frustumCulling;
		CollisionResolver collisionResolver;
		ConstraintSolver constraintSolver;

		SystemManagerNew systemManager;

		SystemAudio audioSystem;
		SystemPhysics physicsSystem;
		SystemPathfinding pathfindingSystem;
		SystemParticleUpdater particleUpdater;
		SystemUIMouseInteraction uiInteract;
		SystemStateMachineUpdater stateUpdater;
		SystemSkeletalAnimationUpdater animSystem;
		SystemBuildMeshList meshListSystem;
		SystemAnimatedGeometryAABBGeneration animAABBSystem;
		SystemLighting lightingSystem;

		SystemCollisionAABB aabbSystem;
		SystemCollisionBox boxSystem;
		SystemCollisionBoxAABB boxAABBSystem;
		SystemCollisionSphere sphereSystem;
		SystemCollisionSphereAABB sphereAABBSystem;
		SystemCollisionSphereBox sphereBoxSystem;

		SystemReflectionBaking reflectionBakingSystem;

		int SCR_WIDTH;
		int SCR_HEIGHT;

		std::string name;

		Camera* camera;

		void BakeReflectionProbes(const bool discardUnfilteredCapture = true) { reflectionBakingSystem.Run(&ecs, &lightManager, discardUnfilteredCapture); }
	};
}