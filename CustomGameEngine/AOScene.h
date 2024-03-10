#pragma once
#include "Scene.h"
namespace Engine {
	class AOScene : public Scene
	{
	private:
		//EntityManager entityManager;
		//SystemManager systemManager;

		bool SSAO;
		void CreateEntities();
		void CreateSystems();
	public:
		AOScene(SceneManager* sceneManager);
		~AOScene();

		void ChangePostProcessEffect();
		void ToggleSSAO();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;
	};
}