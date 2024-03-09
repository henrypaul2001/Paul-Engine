#pragma once
#include "Scene.h"
namespace Engine
{
	class GameScene : public Scene
	{
	private:
		//EntityManager entityManager;
		//SystemManager systemManager;

		bool SSAO;
		void CreateEntities();
		void CreateSystems();
	public:
		GameScene(SceneManager* sceneManager);
		~GameScene();

		void ChangePostProcessEffect();
		void ToggleSSAO();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;
	};
}