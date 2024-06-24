#include "MainMenu.h"
#include "GameInputManager.h"
#include "SystemUIRender.h"
#include "SystemUIMouseInteraction.h"
#include "GameSceneManager.h"
namespace Engine {
	MainMenu::MainMenu(SceneManager* sceneManager) : Scene(sceneManager)
	{
		inputManager = new GameInputManager(this, true);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->DisableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
	}

	MainMenu::~MainMenu()
	{

	}

	void MainMenu::ChangePostProcessEffect()
	{
		SystemRender* renderSystem = dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS));
		unsigned int currentEffect = renderSystem->GetPostProcess();
		unsigned int nextEffect;
		if (currentEffect == 8u) {
			nextEffect = 0u;
		}
		else {
			nextEffect = currentEffect + 1;
		}

		dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS))->SetPostProcess((PostProcessingEffect)nextEffect);
	}

	void MainMenu::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);
	}

	void MainMenu::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void MainMenu::Close()
	{
		std::cout << "Closing menu scene" << std::endl;
	}

	void MainMenu::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();

		inputManager->SetCursorLock(false);
	}

	void MainMenu::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
	}

	void MainMenu::keyDown(int key)
	{

	}

	void MainMenu::PBRButtonRelease(UIButton* button) {
		std::cout << "Launch PBR Scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading PBR Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_PBR);
	}

	void MainMenu::PhysicsButtonRelease(UIButton* button)
	{
		std::cout << "Launch physics scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Physics Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_PHYSICS);
	}

	void MainMenu::InstancingButtonRelease(UIButton* button)
	{
		std::cout << "Launch instancing scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Instancing Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_INSTANCED);
	}

	void MainMenu::GameSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch game scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Game Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_GAME);
	}

	void MainMenu::AudioSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch audio scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Audio Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_AUDIO);
	}

	void MainMenu::ParticleSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch particle scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Particle Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_PARTICLES);
	}

	void MainMenu::AnimationSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch Animation scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Animation Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_ANIMATION);
	}

	void MainMenu::ButtonPress(UIButton* button)
	{

	}

	void MainMenu::ButtonEnter(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(1.0f, 1.0f, 1.0f));
	}

	void MainMenu::ButtonExit(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));
	}

	void MainMenu::CreateEntities()
	{
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Main Menu"), glm::vec2((SCR_WIDTH / 2.0f) - 175.0f, SCR_HEIGHT * 0.8f), glm::vec2(0.5f, 0.5f), font, glm::vec3(1.0f, 1.0f, 1.0f)));
		
		UITextButton* pbrButton = new UITextButton(std::string("PBR Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 145.0f, SCR_HEIGHT * 0.65f), glm::vec2(0.4f, 0.4f), glm::vec2(300.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		pbrButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		pbrButton->SetMouseUpCallback(std::bind(&MainMenu::PBRButtonRelease, this, std::placeholders::_1));
		pbrButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		pbrButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(pbrButton);

		UITextButton* physicsButton = new UITextButton(std::string("Physics Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 180.0f, SCR_HEIGHT * 0.55f), glm::vec2(0.4f, 0.4f), glm::vec2(400.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		physicsButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		physicsButton->SetMouseUpCallback(std::bind(&MainMenu::PhysicsButtonRelease, this, std::placeholders::_1));
		physicsButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		physicsButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(physicsButton);

		UITextButton* instancingButton = new UITextButton(std::string("Instancing Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 210.0f, SCR_HEIGHT * 0.45f), glm::vec2(0.4f, 0.4f), glm::vec2(450.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		instancingButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		instancingButton->SetMouseUpCallback(std::bind(&MainMenu::InstancingButtonRelease, this, std::placeholders::_1));
		instancingButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		instancingButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(instancingButton);

		UITextButton* gameButton = new UITextButton(std::string("Game Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 165.0f, SCR_HEIGHT * 0.35f), glm::vec2(0.4f, 0.4f), glm::vec2(350.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		gameButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		gameButton->SetMouseUpCallback(std::bind(&MainMenu::GameSceneButtonRelease, this, std::placeholders::_1));
		gameButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		gameButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(gameButton);

		UITextButton* audioButton = new UITextButton(std::string("Audio Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 155.0f, SCR_HEIGHT * 0.25f), glm::vec2(0.4f, 0.4f), glm::vec2(350.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		audioButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		audioButton->SetMouseUpCallback(std::bind(&MainMenu::AudioSceneButtonRelease, this, std::placeholders::_1));
		audioButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		audioButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(audioButton);

		UITextButton* particleButton = new UITextButton(std::string("Particle Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 175.0f, SCR_HEIGHT * 0.15f), glm::vec2(0.4f, 0.4f), glm::vec2(375.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		particleButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		particleButton->SetMouseUpCallback(std::bind(&MainMenu::ParticleSceneButtonRelease, this, std::placeholders::_1));
		particleButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		particleButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(particleButton);

		UITextButton* animationButton = new UITextButton(std::string("Animation Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 200.0f, SCR_HEIGHT * 0.05f), glm::vec2(0.4f, 0.4f), glm::vec2(450.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		animationButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		animationButton->SetMouseUpCallback(std::bind(&MainMenu::AnimationSceneButtonRelease, this, std::placeholders::_1));
		animationButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		animationButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->GetUICanvasComponent()->AddUIElement(animationButton);

		entityManager->AddEntity(canvas);
	}

	void MainMenu::CreateSystems()
	{
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIMouseInteraction(inputManager), UPDATE_SYSTEMS);
	}
}