#pragma once
#include "Scene.h"
namespace Engine {
	class InstanceScene : public Scene
	{
	public:
		InstanceScene(SceneManager* sceneManager);
		~InstanceScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

		const std::string& GetName() override { return "InstanceScene"; }

	private:
		void CreateEntities();
		void CreateSystems();
	};
}