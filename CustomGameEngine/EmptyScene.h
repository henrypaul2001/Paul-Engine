#pragma once
#include "Scene.h"
#include "EntityManagerNew.h"
namespace Engine
{

	struct TestComponentA {
		float x, y, z;
	};

	struct TestComponentB {
		float velocityX, velocityY, velocityZ;
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

			ecs.AddComponent<TestComponentA>(0, TestComponentA());
			ecs.AddComponent<TestComponentA>(42, TestComponentA());
			ecs.AddComponent<TestComponentB>(0, TestComponentB());

			bool hasA = ecs.HasComponent<TestComponentA>(0);
			bool doesntHaveA = ecs.HasComponent<TestComponentA>(1);
			bool hasB = ecs.HasComponent<TestComponentB>(0);
		}

		void keyUp(int key) override {}
		void keyDown(int key) override {}

	};
}