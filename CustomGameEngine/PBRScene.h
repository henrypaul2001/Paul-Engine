#pragma once
#include "Scene.h"
#include "UITextButton.h"
namespace Engine {
	class PBRScene : public Scene
	{
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

		void EnvMapBtnRelease(UIButton* button);
		void ParameterGroupRelease(UIButton* button);
		void ParameterIncreaseOptionHold(UIButton* button);
		void ParameterDecreaseOptionHold(UIButton* button);
	private:
		void CreateEntities();
		void CreateSystems();

		int currentEnvMapIndex;
		std::vector<std::string> envMaps;

		std::vector<UIElement*> optionButtons;

		int parameterGroupIndex;
		std::vector<std::vector<UIElement*>> parameterGroups;

		ComponentLight* directionalLight;
		float ambientStrength;

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