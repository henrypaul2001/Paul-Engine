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
		void PhysicsButtonRelease(UIButton* button);
		void InstancingButtonRelease(UIButton* button);
		void GameSceneButtonRelease(UIButton* button);
		void AudioSceneButtonRelease(UIButton* button);
		void ParticleSceneButtonRelease(UIButton* button);
		void AnimationSceneButtonRelease(UIButton* button);
		void SponzaSceneButtonRelease(UIButton* button);
		void AISceneButtonRelease(UIButton* button);

		void ButtonPress(UIButton* button);
		void ButtonEnter(UIButton* button);
		void ButtonExit(UIButton* button);

	private:
		void CreateEntities();
		void CreateSystems();
	};
}