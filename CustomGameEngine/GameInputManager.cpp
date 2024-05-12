#include "GameInputManager.h"
#include "Scene.h"
#include "LightManager.h"
//#include "RenderManager.h"
namespace Engine {
	GameInputManager::GameInputManager(Scene* owner)
	{
		this->owner = owner;
	}

	GameInputManager::~GameInputManager()
	{
		Close();
	}

	void GameInputManager::ProcessInputs()
	{
		if (keysPressed[GLFW_KEY_ESCAPE]) {
			std::cout << "GAMEINPUTMANAGER::KEYCHECK::ESCAPE::TRUE" << std::endl;
			glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
		}

		if (cursorLocked) {
			if (keysPressed[GLFW_KEY_W]) {
				camera->ProcessKeyboard(FORWARD, Scene::dt);
			}
			if (keysPressed[GLFW_KEY_S]) {
				camera->ProcessKeyboard(BACKWARD, Scene::dt);
			}
			if (keysPressed[GLFW_KEY_D]) {
				camera->ProcessKeyboard(RIGHT, Scene::dt);
			}
			if (keysPressed[GLFW_KEY_A]) {
				camera->ProcessKeyboard(LEFT, Scene::dt);
			}

			if (keysPressed[GLFW_KEY_LEFT_CONTROL] && keysPressed[GLFW_KEY_SPACE]) {
				camera->ProcessKeyboard(UP, Scene::dt);
			}
			else if (keysPressed[GLFW_KEY_SPACE]) {
				camera->ProcessKeyboard(UP_WORLD, Scene::dt);
			}

			if (keysPressed[GLFW_KEY_LEFT_CONTROL] && keysPressed[GLFW_KEY_LEFT_SHIFT]) {
				camera->ProcessKeyboard(DOWN, Scene::dt);
			}
			else if (keysPressed[GLFW_KEY_LEFT_SHIFT]) {
				camera->ProcessKeyboard(DOWN_WORLD, Scene::dt);
			}
		}

		if (keysPressed[GLFW_KEY_LEFT]) {
			// Min shadow bias decrease
			dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MinShadowBias -= 0.00005;
			std::cout << "Min shadow bias = " << dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MinShadowBias << std::endl;
		}
		else if (keysPressed[GLFW_KEY_RIGHT]) {
			// Min shadow bias increase
			dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MinShadowBias += 0.00005;
			std::cout << "Min shadow bias = " << dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MinShadowBias << std::endl;
		}

		if (keysPressed[GLFW_KEY_UP]) {
			// Max shadow bias increase
			dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MaxShadowBias += 0.00005;
			std::cout << "Max shadow bias = " << dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MaxShadowBias << std::endl;
		}
		else if (keysPressed[GLFW_KEY_DOWN]) {
			// Max shadow bias decrease
			dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MaxShadowBias -= 0.00005;
			std::cout << "Max shadow bias = " << dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT))->MaxShadowBias << std::endl;
		}

		if (keysPressed[GLFW_KEY_E] && keysPressed[GLFW_KEY_LEFT_CONTROL]) {
			RenderManager::GetInstance()->exposure -= 0.01f;
			std::cout << "Exposure: " << RenderManager::GetInstance()->exposure << std::endl;
		}
		else if (keysPressed[GLFW_KEY_E]) {
			RenderManager::GetInstance()->exposure += 0.01f;
			std::cout << "Exposure: " << RenderManager::GetInstance()->exposure << std::endl;
		}

		if (keysPressed[GLFW_KEY_T] && keysPressed[GLFW_KEY_LEFT_CONTROL]) {
			RenderManager::GetInstance()->bloomThreshold -= 0.1f;
			std::cout << "Bloom threshold: " << RenderManager::GetInstance()->bloomThreshold << std::endl;
		}
		else if (keysPressed[GLFW_KEY_T] && keysPressed[GLFW_KEY_RIGHT_CONTROL]) {
			RenderManager::GetInstance()->bloomThreshold += 0.1f;
			std::cout << "Bloom threshold: " << RenderManager::GetInstance()->bloomThreshold << std::endl;
		}
	}

	void GameInputManager::Close()
	{
		// clear scene manager delegates
	}

	void GameInputManager::keyUp(int key)
	{
		std::cout << "Key num: " << key << "| UP" << std::endl;
		owner->keyUp(key);
		RenderManager* renderInstance = RenderManager::GetInstance();
		if (key == GLFW_KEY_B) {
			bool bloom = (renderInstance->GetRenderOptions() & RENDER_BLOOM) != 0;
			if (bloom) {
				renderInstance->DisableRenderOptions(RENDER_BLOOM);
				std::cout << "GAMEINPUTMANAGER::DISABLE::Bloom" << std::endl;
			}
			else {
				renderInstance->EnableRenderOptions(RENDER_BLOOM);
				std::cout << "GAMEINPUTMANAGER::ENABLE::Bloom" << std::endl;
			}
		}
		if (key == GLFW_KEY_P) {
			bool SSAO = (renderInstance->GetRenderOptions() & RENDER_SSAO) != 0;
			if (SSAO) {
				renderInstance->DisableRenderOptions(RENDER_SSAO);
				std::cout << "GAMEINPUTMANAGER::DISABLE::Screen space ambient occlusion" << std::endl;
			}
			else {
				renderInstance->EnableRenderOptions(RENDER_SSAO);
				std::cout << "GAMEINPUTMANAGER::ENABLE::Screen space ambient occlusion" << std::endl;
			}
		}
		if (key == GLFW_KEY_C) {
			SetCursorLock(!GetCursorLock());
		}
	}

	void GameInputManager::keyDown(int key)
	{
		std::cout << "Key num: " << key << "| DOWN" << std::endl;
		owner->keyDown(key);
	}

	void GameInputManager::mouseUp(int button)
	{
		std::cout << "Button num: " << button << "| DOWN" << std::endl;
	}

	void GameInputManager::mouseDown(int button)
	{
		std::cout << "Button num: " << button << "| UP" << std::endl;
	}
}