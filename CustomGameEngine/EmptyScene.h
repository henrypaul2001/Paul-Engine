#pragma once
#include "Scene.h"
#include "EntityManagerNew.h"

#include "NavigationGrid.h"

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

#include "SystemCollisionAABB.h"
#include "SystemCollisionBox.h"
#include "SystemCollisionBoxAABB.h"
#include "SystemCollisionSphere.h"
#include "SystemCollisionSphereAABB.h"
#include "SystemCollisionsphereBox.h"

namespace Engine
{
	struct TestComponentA {
		float x, y, z;
	};

	struct TestComponentB {
		float velocityX, velocityY, velocityZ;
	};

	struct TestComponentC {
		float c;
	};

	struct TestComponentD {
		bool d;
	};

	struct TestComponentE {
		long e;
	};

	struct TestComponentF {
		long long f;
	};

	static void TestSystem(const unsigned int entityID, ComponentTransform& transform, ComponentPhysics& physics) {
		std::cout << "TEST_SYSTEM: EntityID = " << entityID << std::endl;
	}
	static void TestAfterAction() {
		std::cout << "TEST_SYSTEM: After action" << std::endl;
	}

	class EmptyScene : public Scene
	{
	public:
		EmptyScene(SceneManager* sceneManager);
		~EmptyScene() {}

		void Update() override {
			systemManager.ActionPreUpdateSystems();
			Scene::Update();
			systemManager.ActionSystems();
		}
		void Render() override {
			Scene::Render();
		}
		void Close() override {}
		void SetupScene() override {
			SkeletalAnimation* vampireDanceAnim = ResourceManager::GetInstance()->LoadAnimation("Models/vampire/dancing_vampire.dae");

			navGrid = NavigationGrid("Data/NavigationGrid/TestGrid1.txt");
			AudioFile* campfireCrackling = ResourceManager::GetInstance()->LoadAudio("Audio/campfire.wav", 1.0f, 0.0f, 2.0f);

			// All components entity
			EntityNew* allComponents = ecs.New("All Components");
			ecs.AddComponent(allComponents->ID(), ComponentPhysics());
			ecs.AddComponent(allComponents->ID(), ComponentAnimator(vampireDanceAnim));
			ecs.AddComponent(allComponents->ID(), ComponentAudioSource(campfireCrackling));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionSphere(10.0f));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionAABB(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionBox(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
			ecs.AddComponent(allComponents->ID(), ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
			ecs.AddComponent(allComponents->ID(), ComponentLight(SPOT));
			ecs.AddComponent(allComponents->ID(), ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false)));
			ecs.AddComponent(allComponents->ID(), ComponentPathfinder(&navGrid));
			ecs.AddComponent(allComponents->ID(), ComponentStateController());
			ecs.AddComponent(allComponents->ID(), ComponentUICanvas(SCREEN_SPACE));

			for (int i = 0; i < 50; i++) {
				ecs.New("Test");
			}
			ecs.New("Hello");

			const EntityNew* missingEntity = ecs.Find("I don't exist");
			const EntityNew* foundEntity = ecs.Find("Hello");
			const EntityNew* duplicateFoundEntity = ecs.Find("Test (10)");

			EntityNew* findTen = ecs.Find(10);

			ecs.Delete(*findTen);
			ecs.Delete(*ecs.Find(5));

			EntityNew* deletedTen = ecs.Find("Test (10)");
			EntityNew* deletedFive = ecs.Find("Test (5)");

			ecs.New("After delete");

			bool equalsFalse = ecs.Delete("I don't exist");
			bool equalsTrue = ecs.Delete("After delete");

			ecs.RegisterComponentType<TestComponentA>();
			ecs.RegisterComponentType<TestComponentB>();

			ecs.AddComponent(0, TestComponentA());
			ecs.AddComponent(42, TestComponentA());
			ecs.AddComponent(0, TestComponentB());

			bool hasA = ecs.HasComponent<TestComponentA>(0);
			bool doesntHaveA = ecs.HasComponent<TestComponentA>(1);
			bool hasB = ecs.HasComponent<TestComponentB>(0);

			std::bitset<MAX_COMPONENTS> testMask = ecs.CreateMask<TestComponentB>();

			ecs.RemoveComponent<TestComponentA, TestComponentB>(0);
			ecs.RemoveComponent<TestComponentA>(42);

			//ecs.AddComponent(2, TestComponentA());
			ecs.AddComponent(0, TestComponentA());
			ecs.AddComponent(2, TestComponentB());

			TestComponentB* bComponent = ecs.GetComponent<TestComponentB>(2);
			bComponent->velocityX = 10.0f;
			TestComponentA* nullpointer = ecs.GetComponent<TestComponentA>(2);

			ecs.Delete(2);

			std::cout << bComponent->velocityX << std::endl;
		
			EntityNew* clone_base = ecs.New("Clone base");
			TestComponentB b;
			b.velocityX = -2.5f;
			b.velocityY = 5.0f;
			ecs.AddComponent(clone_base->ID(), TestComponentA());
			ecs.AddComponent(clone_base->ID(), b);

			EntityNew* cloned = ecs.Clone(*clone_base);

			for (int i = 0; i < 50; i++) {
				EntityNew* entity = ecs.New("ViewTest");
				TestComponentA a;
				a.x = i;
				ecs.AddComponent(entity->ID(), a);
				if (i % 2 == 0) {
					TestComponentB b;
					b.velocityY = i;
					ecs.AddComponent(entity->ID(), b);
				}
				if (i % 3 == 0) {
					TestComponentC c;
					c.c = i;
					ecs.AddComponent(entity->ID(), c);
				}
				if (i % 4 == 0) {
					TestComponentF f;
					f.f = i;
					ecs.AddComponent(entity->ID(), f);
					ecs.AddComponent(entity->ID(), ComponentPhysics());
				}
			}

			View<TestComponentA, TestComponentB> testView = ecs.View<TestComponentA, TestComponentB>();
			testView.ForEach([](const unsigned int entityID, TestComponentA& a, TestComponentB& b) {
				unsigned int id = entityID;
				float x = a.x;
				float velocityY = b.velocityY;
			});

			View<TestComponentF> fView = ecs.View<TestComponentF>();
			fView.ForEach([](const unsigned int entityID, TestComponentF& f) {
				unsigned int id = entityID;
				if (id == 10) {
					f.f = 3000.0;
				}
			});

			std::cout << ecs.GetComponent<TestComponentF>(10)->f << std::endl;

			EntityNew* transformTest = ecs.New("Transform Test");
			ecs.AddComponent(transformTest->ID(), ComponentTransform(&ecs, 10.0f, 1.0f, -5.0f));
			//ecs.RemoveComponent<ComponentTransform>(transformTest->ID()); // build error, cannot remove transform component

			// Transform children
			for (int i = 0; i < 20; i++) {
				EntityNew* child = ecs.New("Transform Test");
				ecs.AddComponent(child->ID(), ComponentTransform(&ecs, 10.0f + i, 1.0f + i, -5.0f - i));
				ecs.GetComponent<ComponentTransform>(transformTest->ID())->AddChild(child);
			}

			ComponentTransform* transformChild = ecs.GetComponent<ComponentTransform>(ecs.GetComponent<ComponentTransform>(transformTest->ID())->FindChildWithName("Transform Test (3)")->ID());

			// Systems
			systemManager.RegisterPreUpdateSystem(animAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&, ComponentAnimator&)>(std::bind(&SystemAnimatedGeometryAABBGeneration::OnAction, &animAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)), []() {}, std::bind(&SystemAnimatedGeometryAABBGeneration::AfterAction, &animAABBSystem));
			systemManager.RegisterPreUpdateSystem(meshListSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentGeometry&)>(std::bind(&SystemBuildMeshList::OnAction, &meshListSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), std::bind(&SystemBuildMeshList::PreAction, &meshListSystem));
			 
			//systemManager.RegisterSystem("TEST_SYSTEM", std::function(TestSystem), &TestAfterAction);
			systemManager.RegisterPreUpdateSystem(aabbSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionAABB&)>(std::bind(&SystemCollisionAABB::OnAction, &aabbSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionAABB::AfterAction, &aabbSystem));
			systemManager.RegisterPreUpdateSystem(boxSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionBox&)>(std::bind(&SystemCollisionBox::OnAction, &boxSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionBox::AfterAction, &boxSystem));
			systemManager.RegisterPreUpdateSystem(boxAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionBox&)>(std::bind(&SystemCollisionBoxAABB::OnAction, &boxAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionBoxAABB::AfterAction, &boxAABBSystem));
			systemManager.RegisterPreUpdateSystem(sphereSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphere::OnAction, &sphereSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphere::AfterAction, &sphereSystem));
			systemManager.RegisterPreUpdateSystem(sphereAABBSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphereAABB::OnAction, &sphereAABBSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphereAABB::AfterAction, &sphereAABBSystem));
			systemManager.RegisterPreUpdateSystem(sphereBoxSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentCollisionSphere&)>(std::bind(&SystemCollisionSphereBox::OnAction, &sphereBoxSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemCollisionSphereBox::AfterAction, &sphereBoxSystem));

			systemManager.RegisterSystem(audioSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentAudioSource&)>(std::bind(&SystemAudio::OnAction, &audioSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), [](){}, std::bind(&SystemAudio::AfterAction, &audioSystem));
			systemManager.RegisterSystem(physicsSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentPhysics&)>(std::bind(&SystemPhysics::OnAction, &physicsSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemPhysics::AfterAction, &physicsSystem));
			systemManager.RegisterSystem(pathfindingSystem.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentPathfinder&)>(std::bind(&SystemPathfinding::OnAction, &pathfindingSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemPathfinding::AfterAction, &pathfindingSystem));
			systemManager.RegisterSystem(particleUpdater.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentParticleGenerator&)>(std::bind(&SystemParticleUpdater::OnAction, &particleUpdater, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemParticleUpdater::AfterAction, &particleUpdater));
			systemManager.RegisterSystem(uiInteract.SystemName(), std::function<void(const unsigned int, ComponentTransform&, ComponentUICanvas&)>(std::bind(&SystemUIMouseInteraction::OnAction, &uiInteract, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemUIMouseInteraction::AfterAction, &uiInteract));
			systemManager.RegisterSystem(stateUpdater.SystemName(), std::function<void(const unsigned int, ComponentStateController&)>(std::bind(&SystemStateMachineUpdater::OnAction, &stateUpdater, std::placeholders::_1, std::placeholders::_2)), []() {}, std::bind(&SystemStateMachineUpdater::AfterAction, &stateUpdater));
			systemManager.RegisterSystem(animSystem.SystemName(), std::function<void(const unsigned int, ComponentGeometry&, ComponentAnimator&)>(std::bind(&SystemSkeletalAnimationUpdater::OnAction, &animSystem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)), []() {}, std::bind(&SystemSkeletalAnimationUpdater::AfterAction, &animSystem));
		}

		void keyUp(int key) override {}
		void keyDown(int key) override {}

	private:
		NavigationGrid navGrid;

		EntityManagerNew ecs;
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

		SystemCollisionAABB aabbSystem;
		SystemCollisionBox boxSystem;
		SystemCollisionBoxAABB boxAABBSystem;
		SystemCollisionSphere sphereSystem;
		SystemCollisionSphereAABB sphereAABBSystem;
		SystemCollisionSphereBox sphereBoxSystem;
	};
}