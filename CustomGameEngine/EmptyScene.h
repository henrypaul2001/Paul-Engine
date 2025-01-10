#pragma once
#include "Scene.h"
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
		void SetupScene() override {}

		void keyUp(int key) override {}
		void keyDown(int key) override {}

	};
}