#pragma once
#include "Scene.h"
#include "EntityManagerNew.h"
namespace Engine
{
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
		}

		void keyUp(int key) override {}
		void keyDown(int key) override {}

	};
}