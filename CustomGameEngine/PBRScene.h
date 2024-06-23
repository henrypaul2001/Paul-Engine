#pragma once
#include "Scene.h"
#include "UITextButton.h"
namespace Engine {
	class PBRScene : public Scene
	{
		void CreateEntities();
		void CreateSystems();
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
		int currentEnvMapIndex;
		std::vector<HDREnvironment*> envMaps;

		std::vector<UIElement*> optionButtons;

		int parameterGroupIndex;
		std::vector<std::vector<UIElement*>> parameterGroups;

		ComponentLight* directionalLight;
		float ambientStrength;
	};
}