#pragma once
#include "Scene.h"
#include "GameInputManager.h"
#include "SystemUIRender.h"
namespace Engine {
    class IBLScene : public Scene
    {
    public:
        IBLScene(SceneManager* sceneManager);
        ~IBLScene();

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
    };
}