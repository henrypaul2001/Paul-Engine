#pragma once
#include "Scene.h"
namespace Engine {
	class SponzaScene : public Scene
	{
	private:
		void CreateEntities();
		void CreateSystems();
	public:
		SponzaScene(SceneManager* sceneManager);
		~SponzaScene();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

	};
}