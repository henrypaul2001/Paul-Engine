#pragma once
#include "Scene.h"
namespace Engine {
    class SSRScene : public Scene
    {
    public:
        SSRScene(SceneManager* sceneManager);
		~SSRScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;
	private:
		void CreateEntities();

		std::unordered_map<unsigned int, std::string> postProcessEffectToString = {
			{ 0u, std::string("None") },
			{ 1u, std::string("Inverted") },
			{ 2u, std::string("Grayscale") },
			{ 3u, std::string("Hard Sharpen") },
			{ 4u, std::string("Subtle Sharpen") },
			{ 5u, std::string("Blur") },
			{ 6u, std::string("Edge Detect") },
			{ 7u, std::string("Emboss") },
			{ 8u, std::string("Sobel") }
		};
    };
}