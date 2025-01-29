#pragma once
#include "Scene.h"
#include "UIText.h"
#include "UIImage.h"
#include "UITextButton.h"
#include "UIImageButton.h"
namespace Engine {
	class SponzaScene : public Scene
	{
	public:
		SponzaScene(SceneManager* sceneManager);
		~SponzaScene();

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