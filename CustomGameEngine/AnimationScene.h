#pragma once
#include "Scene.h"
namespace Engine {
	class AnimationScene : public Scene
	{
	public:
		AnimationScene(SceneManager* sceneManager);
		~AnimationScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

		const std::string& GetName() override { return "AnimationScene"; }
	private:
		void CreateEntities();
		void CreateSystems();

		int animIndex;
		std::vector<SkeletalAnimation*> animations;
	};
}