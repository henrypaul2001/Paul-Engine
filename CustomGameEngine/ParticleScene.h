#pragma once
#include "Scene.h"
namespace Engine {
	class ParticleScene : public Scene
	{
	public:
		ParticleScene(SceneManager* sceneManager);
		~ParticleScene();
		
		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;

		const std::string& GetName() override { return "ParticleScene"; }
	private:
		void CreateEntities();
		void CreateSystems();
	};
}