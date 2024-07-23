#pragma once
#include "Scene.h"
namespace Engine {
	class CollisionScene : public Scene
	{
		void CreateEntities();
		void CreateSystems();
	public:
		CollisionScene(SceneManager* sceneManager);
		~CollisionScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

		const std::string& GetName() override { return "CollisionScene"; }
	};
}