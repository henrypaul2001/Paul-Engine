#pragma once
#include "Scene.h"
#include "UIText.h"
#include "UITextButton.h"
namespace Engine {
	class MainMenu : public Scene
	{
	public:
		MainMenu(SceneManager* sceneManager);
		~MainMenu();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

		void PBRButtonRelease(UIButton* button);
	private:
		void CreateEntities();
		void CreateSystems();
	};
}