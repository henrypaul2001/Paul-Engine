#pragma once
#include "Scene.h"
#include "UITextButton.h"
namespace Engine {
	class PBRScene : public Scene
	{
		void CreateEntities();
		void CreateSystems();
	public:
		PBRScene(SceneManager* sceneManager);
		~PBRScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;
	};
}