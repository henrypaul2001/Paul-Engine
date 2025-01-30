#pragma once
#include "Scene.h"
#include "UIText.h"
#include "UIImage.h"
#include "UITextButton.h"
#include "UIImageButton.h"
namespace Engine {
	class GeoCullingScene : public Scene
	{
	public:
		GeoCullingScene(SceneManager* sceneManager);
		~GeoCullingScene();

		void ChangePostProcessEffect();

		void Update() override;
		void Render() override;
		void Close() override;
		void SetupScene() override;

		void keyUp(int key) override;
		void keyDown(int key) override;
	private:
		void CreateEntities();
		void CreateSystems();

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