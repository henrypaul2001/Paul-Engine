#pragma once
#include "Scene.h"
namespace Engine {
	class AOScene : public Scene
	{
	public:
		AOScene(SceneManager* sceneManager);
		~AOScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

	private:
		void CreateEntities();
		void CreateSystems();
	};
}