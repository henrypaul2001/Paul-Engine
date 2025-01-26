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
	enum DefaultSystemType {
		SYSTEM_ANIMATED_GEOBOUNDS,
		SYSTEM_BUILD_MESH_LIST,
		SYSTEM_COLLISION_AABB,
		SYSTEM_COLLISION_BOX,
		SYSTEM_COLLISION_BOX_AABB,
		SYSTEM_COLLISION_SPHERE,
		SYSTEM_COLLISION_SPHERE_AABB,
		SYSTEM_COLLISION_SPHERE_BOX,
		SYSTEM_AUDIO,
		SYSTEM_PHYSICS,
		SYSTEM_PATHFINDING,
		SYSTEM_PARTICLE_UPDATE,
		SYSTEM_UI_INTERACT,
		SYSTEM_STATE_MACHINE_UPDATE,
		SYSTEM_ANIMATION,
		SYSTEM_LIGHTING
	};

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

		void RegisterAllDefaultSystems() {
			RegisterSystemToPreUpdate(SYSTEM_ANIMATED_GEOBOUNDS);
			RegisterSystemToPreUpdate(SYSTEM_BUILD_MESH_LIST);
			RegisterSystemToPreUpdate(SYSTEM_COLLISION_AABB);
			RegisterSystemToPreUpdate(SYSTEM_COLLISION_BOX);
			RegisterSystemToPreUpdate(SYSTEM_COLLISION_BOX_AABB);
			RegisterSystemToPreUpdate(SYSTEM_COLLISION_SPHERE);
			RegisterSystemToPreUpdate(SYSTEM_COLLISION_SPHERE_AABB);
			RegisterSystemToPreUpdate(SYSTEM_COLLISION_SPHERE_BOX);

			RegisterSystem(SYSTEM_AUDIO);
			RegisterSystem(SYSTEM_PHYSICS);
			RegisterSystem(SYSTEM_PATHFINDING);
			RegisterSystem(SYSTEM_PARTICLE_UPDATE);
			RegisterSystem(SYSTEM_UI_INTERACT);
			RegisterSystem(SYSTEM_STATE_MACHINE_UPDATE);
			RegisterSystem(SYSTEM_ANIMATION);
			RegisterSystem(SYSTEM_LIGHTING);
		}

		void RegisterSystemToPreUpdate(const DefaultSystemType systemType) {
			switch (systemType) {
			case SYSTEM_ANIMATED_GEOBOUNDS:
				systemManager.RegisterPreUpdateSystem(animAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&, ComponentAnimator&)>(std::bind(&SystemAnimatedGeometryAABBGeneration::OnAction, &animAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)), []() {}, std::bind(&SystemAnimatedGeometryAABBGeneration::AfterAction, &animAABBSystem));
				break;
			case SYSTEM_BUILD_MESH_LIST:
				systemManager.RegisterPreUpdateSystem(meshListSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&)>(std::bind(&SystemBuildMeshList::OnAction, &meshListSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), std::bind(&SystemBuildMeshList::PreAction, &meshListSystem));
				break;
			case SYSTEM_COLLISION_AABB:
				systemManager.RegisterPreUpdateSystem(aabbSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionAABB&)>(std::bind(&SystemCollisionAABB::OnAction, &aabbSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionAABB::AfterAction, &aabbSystem));
				break;
			case SYSTEM_COLLISION_BOX:
				systemManager.RegisterPreUpdateSystem(boxSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionBox&)>(std::bind(&SystemCollisionBox::OnAction, &boxSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionBox::AfterAction, &boxSystem));
				break;
			case SYSTEM_COLLISION_BOX_AABB:
				systemManager.RegisterPreUpdateSystem(boxAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionBox&)>(std::bind(&SystemCollisionBoxAABB::OnAction, &boxAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionBoxAABB::AfterAction, &boxAABBSystem));
				break;
			case SYSTEM_COLLISION_SPHERE:
				systemManager.RegisterPreUpdateSystem(sphereSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphere::OnAction, &sphereSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphere::AfterAction, &sphereSystem));
				break;
			case SYSTEM_COLLISION_SPHERE_AABB:
				systemManager.RegisterPreUpdateSystem(sphereAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphereAABB::OnAction, &sphereAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphereAABB::AfterAction, &sphereAABBSystem));
				break;
			case SYSTEM_COLLISION_SPHERE_BOX:
				systemManager.RegisterPreUpdateSystem(sphereBoxSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphereBox::OnAction, &sphereBoxSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphereBox::AfterAction, &sphereBoxSystem));
				break;
			case SYSTEM_AUDIO:
				systemManager.RegisterPreUpdateSystem(audioSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentAudioSource&)>(std::bind(&SystemAudio::OnAction, &audioSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemAudio::AfterAction, &audioSystem));
				break;
			case SYSTEM_PHYSICS:
				systemManager.RegisterPreUpdateSystem(physicsSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentPhysics&)>(std::bind(&SystemPhysics::OnAction, &physicsSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemPhysics::AfterAction, &physicsSystem));
				break;
			case SYSTEM_PATHFINDING:
				systemManager.RegisterPreUpdateSystem(pathfindingSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentPathfinder&)>(std::bind(&SystemPathfinding::OnAction, &pathfindingSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemPathfinding::AfterAction, &pathfindingSystem));
				break;
			case SYSTEM_PARTICLE_UPDATE:
				systemManager.RegisterPreUpdateSystem(particleUpdater.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentParticleGenerator&)>(std::bind(&SystemParticleUpdater::OnAction, &particleUpdater, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemParticleUpdater::AfterAction, &particleUpdater));
				break;
			case SYSTEM_UI_INTERACT:
				systemManager.RegisterPreUpdateSystem(uiInteract.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentUICanvas&)>(std::bind(&SystemUIMouseInteraction::OnAction, &uiInteract, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemUIMouseInteraction::AfterAction, &uiInteract));
				break;
			case SYSTEM_STATE_MACHINE_UPDATE:
				systemManager.RegisterPreUpdateSystem(stateUpdater.SystemName(), std::function<void(const unsigned int, ComponentStateController&)>(std::bind(&SystemStateMachineUpdater::OnAction, &stateUpdater, std::placeholders::_1, std::placeholders::_2)), []() {}, std::bind(&SystemStateMachineUpdater::AfterAction, &stateUpdater));
				break;
			case SYSTEM_ANIMATION:
				systemManager.RegisterPreUpdateSystem(animSystem.SystemName(), std::function<void(const unsigned int, ComponentGeometry&, ComponentAnimator&)>(std::bind(&SystemSkeletalAnimationUpdater::OnAction, &animSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemSkeletalAnimationUpdater::AfterAction, &animSystem));
				break;
			case SYSTEM_LIGHTING:
				systemManager.RegisterPreUpdateSystem(lightingSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentLight&)>(std::bind(&SystemLighting::OnAction, &lightingSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), std::bind(&SystemLighting::PreAction, &lightingSystem), std::bind(&SystemLighting::AfterAction, &lightingSystem));
				break;
			}
		}
		void RegisterSystem(const DefaultSystemType systemType) {
			switch (systemType) {
			case SYSTEM_ANIMATED_GEOBOUNDS:
				systemManager.RegisterSystem(animAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&, ComponentAnimator&)>(std::bind(&SystemAnimatedGeometryAABBGeneration::OnAction, &animAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)), []() {}, std::bind(&SystemAnimatedGeometryAABBGeneration::AfterAction, &animAABBSystem));
				break;
			case SYSTEM_BUILD_MESH_LIST:
				systemManager.RegisterSystem(meshListSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&)>(std::bind(&SystemBuildMeshList::OnAction, &meshListSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), std::bind(&SystemBuildMeshList::PreAction, &meshListSystem));
				break;
			case SYSTEM_COLLISION_AABB:
				systemManager.RegisterSystem(aabbSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionAABB&)>(std::bind(&SystemCollisionAABB::OnAction, &aabbSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionAABB::AfterAction, &aabbSystem));
				break;
			case SYSTEM_COLLISION_BOX:
				systemManager.RegisterSystem(boxSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionBox&)>(std::bind(&SystemCollisionBox::OnAction, &boxSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionBox::AfterAction, &boxSystem));
				break;
			case SYSTEM_COLLISION_BOX_AABB:
				systemManager.RegisterSystem(boxAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionBox&)>(std::bind(&SystemCollisionBoxAABB::OnAction, &boxAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionBoxAABB::AfterAction, &boxAABBSystem));
				break;
			case SYSTEM_COLLISION_SPHERE:
				systemManager.RegisterSystem(sphereSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphere::OnAction, &sphereSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphere::AfterAction, &sphereSystem));
				break;
			case SYSTEM_COLLISION_SPHERE_AABB:
				systemManager.RegisterSystem(sphereAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphereAABB::OnAction, &sphereAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphereAABB::AfterAction, &sphereAABBSystem));
				break;
			case SYSTEM_COLLISION_SPHERE_BOX:
				systemManager.RegisterSystem(sphereBoxSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphereBox::OnAction, &sphereBoxSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphereBox::AfterAction, &sphereBoxSystem));
				break;
			case SYSTEM_AUDIO:
				systemManager.RegisterSystem(audioSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentAudioSource&)>(std::bind(&SystemAudio::OnAction, &audioSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemAudio::AfterAction, &audioSystem));
				break;
			case SYSTEM_PHYSICS:
				systemManager.RegisterSystem(physicsSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentPhysics&)>(std::bind(&SystemPhysics::OnAction, &physicsSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemPhysics::AfterAction, &physicsSystem));
				break;
			case SYSTEM_PATHFINDING:
				systemManager.RegisterSystem(pathfindingSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentPathfinder&)>(std::bind(&SystemPathfinding::OnAction, &pathfindingSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemPathfinding::AfterAction, &pathfindingSystem));
				break;
			case SYSTEM_PARTICLE_UPDATE:
				systemManager.RegisterSystem(particleUpdater.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentParticleGenerator&)>(std::bind(&SystemParticleUpdater::OnAction, &particleUpdater, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemParticleUpdater::AfterAction, &particleUpdater));
				break;
			case SYSTEM_UI_INTERACT:
				systemManager.RegisterSystem(uiInteract.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentUICanvas&)>(std::bind(&SystemUIMouseInteraction::OnAction, &uiInteract, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemUIMouseInteraction::AfterAction, &uiInteract));
				break;
			case SYSTEM_STATE_MACHINE_UPDATE:
				systemManager.RegisterSystem(stateUpdater.SystemName(), std::function<void(const unsigned int, ComponentStateController&)>(std::bind(&SystemStateMachineUpdater::OnAction, &stateUpdater, std::placeholders::_1, std::placeholders::_2)), []() {}, std::bind(&SystemStateMachineUpdater::AfterAction, &stateUpdater));
				break;
			case SYSTEM_ANIMATION:
				systemManager.RegisterSystem(animSystem.SystemName(), std::function<void(const unsigned int, ComponentGeometry&, ComponentAnimator&)>(std::bind(&SystemSkeletalAnimationUpdater::OnAction, &animSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemSkeletalAnimationUpdater::AfterAction, &animSystem));
				break;
			case SYSTEM_LIGHTING:
				systemManager.RegisterSystem(lightingSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentLight&)>(std::bind(&SystemLighting::OnAction, &lightingSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), std::bind(&SystemLighting::PreAction, &lightingSystem), std::bind(&SystemLighting::AfterAction, &lightingSystem));
				break;
			}
		}
	};
}