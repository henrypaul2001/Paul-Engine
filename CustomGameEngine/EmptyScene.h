#pragma once
#include "Scene.h"
#include "EntityManagerNew.h"

#include "ComponentPhysics.h"
#include "ComponentAnimator.h"
#include "ComponentAudioSource.h"
#include "ComponentCollisionSphere.h"
#include "ComponentCollisionBox.h"
#include "ComponentCollisionAABB.h"
#include "ComponentGeometry.h"


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

	class EmptyScene : public Scene
	{
	private:
		//EntityManager entityManager;
		//SystemManager systemManager;

	public:
		EmptyScene(SceneManager* sceneManager);
		~EmptyScene() {}

		void Update() override {}
		void Render() override {}
		void Close() override {}
		void SetupScene() override {
			EntityManagerNew ecs;
			
			// All components entity
			EntityNew* allComponents = ecs.New("All Components");
			ecs.AddComponent(allComponents->ID(), ComponentPhysics());
			ecs.AddComponent(allComponents->ID(), ComponentAnimator(nullptr));
			ecs.AddComponent(allComponents->ID(), ComponentAudioSource(nullptr));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionSphere(10.0f));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionAABB(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionBox(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
			ecs.AddComponent(allComponents->ID(), ComponentGeometry(MODEL_CUBE));
			ecs.AddComponent(allComponents->ID(), 2.2f);
			ecs.AddComponent(allComponents->ID(), (double)2);
			ecs.AddComponent(allComponents->ID(), TestComponentA());
			ecs.AddComponent(allComponents->ID(), TestComponentB());
			bool shouldBeTrue = ecs.AddComponent(allComponents->ID(), TestComponentC());
			bool shouldBeFalse = ecs.AddComponent(allComponents->ID(), TestComponentD());

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
			testView.ForEach([](unsigned int entityID, TestComponentA& a, TestComponentB& b) {
				unsigned int id = entityID;
				float x = a.x;
				float velocityY = b.velocityY;
			});

			View<TestComponentF> fView = ecs.View<TestComponentF>();
			fView.ForEach([](unsigned int entityID, TestComponentF& f) {
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
		}

		void keyUp(int key) override {}
		void keyDown(int key) override {}

	};
}