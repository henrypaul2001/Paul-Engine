#pragma once
#include "Scene.h"
namespace Engine {
	class AIScene : public Scene
	{
	public:
		AIScene(SceneManager* sceneManager);
		~AIScene();

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