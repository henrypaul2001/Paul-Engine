#include "MainMenu.h"
#include "GameInputManager.h"
#include "GameSceneManager.h"
namespace Engine {
	MainMenu::MainMenu(SceneManager* sceneManager) : Scene(sceneManager, "MainMenu")
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
		SystemRender& renderSystem = renderManager->GetRenderPipeline()->GetRenderSystem();
		unsigned int currentEffect = renderSystem.GetPostProcess();
		unsigned int nextEffect;
		if (currentEffect == 8u) {
			nextEffect = 0u;
		}
		else {
			nextEffect = currentEffect + 1;
		}

		renderSystem.SetPostProcess((PostProcessingEffect)nextEffect);
	}

	void MainMenu::Update()
	{
		systemManager.ActionPreUpdateSystems();
		Scene::Update();
		systemManager.ActionSystems();
	}

	void MainMenu::Render()
	{
		Scene::Render();
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

	void MainMenu::SponzaSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch Sponza scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Sponza Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_SPONZA);
	}

	void MainMenu::AISceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch AI scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading AI Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_AI);
	}

	void MainMenu::IBLSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch IBL scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading IBL Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_IBL);
	}

	void MainMenu::GeoCullingSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch IBL scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading Culling Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_GEO_CULLING);
	}

	void MainMenu::SSRSceneButtonRelease(UIButton* button)
	{
		std::cout << "Launch SSR scene" << std::endl;
		dynamic_cast<UITextButton*>(button)->SetText("Loading SSR Scene");
		button->Position(glm::vec2(button->Position().x - 95.0f, button->Position().y));
		sceneManager->ChangeSceneAtEndOfFrame(SCENE_SSR);
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
		TextFont* font = resources->LoadTextFont("Fonts/arial.ttf");

		Entity* uiCanvas = ecs.New("Canvas");
		ecs.AddComponent(uiCanvas->ID(), ComponentUICanvas(SCREEN_SPACE));
		ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

		UIBackground titleBackground;
		titleBackground.Colour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		titleBackground.LeftRightUpDownExtents = glm::vec4(0.05f, 0.325f, 0.15f, 0.1f);
		titleBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 0.35f);
		titleBackground.BorderThickness = 0.01f;
		titleBackground.Bordered = true;

		canvas->AddUIElement(new UIText(std::string("Main Menu"), glm::vec2((SCR_WIDTH / 2.0f) - 175.0f, SCR_HEIGHT * 0.8f), glm::vec2(0.5f, 0.5f), font, glm::vec3(1.0f, 1.0f, 1.0f), titleBackground));
		
		UITextButton* pbrButton = new UITextButton(std::string("PBR Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 145.0f, SCR_HEIGHT * 0.65f), glm::vec2(0.4f, 0.4f), glm::vec2(300.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		pbrButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		pbrButton->SetMouseUpCallback(std::bind(&MainMenu::PBRButtonRelease, this, std::placeholders::_1));
		pbrButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		pbrButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(pbrButton);

		UITextButton* physicsButton = new UITextButton(std::string("Physics Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 180.0f, SCR_HEIGHT * 0.55f), glm::vec2(0.4f, 0.4f), glm::vec2(400.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		physicsButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		physicsButton->SetMouseUpCallback(std::bind(&MainMenu::PhysicsButtonRelease, this, std::placeholders::_1));
		physicsButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		physicsButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(physicsButton);

		UITextButton* instancingButton = new UITextButton(std::string("Instancing Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 210.0f, SCR_HEIGHT * 0.45f), glm::vec2(0.4f, 0.4f), glm::vec2(450.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		instancingButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		instancingButton->SetMouseUpCallback(std::bind(&MainMenu::InstancingButtonRelease, this, std::placeholders::_1));
		instancingButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		instancingButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(instancingButton);

		UITextButton* gameButton = new UITextButton(std::string("Game Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 165.0f, SCR_HEIGHT * 0.35f), glm::vec2(0.4f, 0.4f), glm::vec2(350.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		gameButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		gameButton->SetMouseUpCallback(std::bind(&MainMenu::GameSceneButtonRelease, this, std::placeholders::_1));
		gameButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		gameButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(gameButton);

		UITextButton* audioButton = new UITextButton(std::string("Audio Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 155.0f, SCR_HEIGHT * 0.25f), glm::vec2(0.4f, 0.4f), glm::vec2(350.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		audioButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		audioButton->SetMouseUpCallback(std::bind(&MainMenu::AudioSceneButtonRelease, this, std::placeholders::_1));
		audioButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		audioButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(audioButton);

		UITextButton* particleButton = new UITextButton(std::string("Particle Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 175.0f, SCR_HEIGHT * 0.15f), glm::vec2(0.4f, 0.4f), glm::vec2(375.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		particleButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		particleButton->SetMouseUpCallback(std::bind(&MainMenu::ParticleSceneButtonRelease, this, std::placeholders::_1));
		particleButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		particleButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(particleButton);

		UITextButton* animationButton = new UITextButton(std::string("Animation Scene"), glm::vec2((SCR_WIDTH / 2.0f) - 200.0f, SCR_HEIGHT * 0.05f), glm::vec2(0.4f, 0.4f), glm::vec2(450.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		animationButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		animationButton->SetMouseUpCallback(std::bind(&MainMenu::AnimationSceneButtonRelease, this, std::placeholders::_1));
		animationButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		animationButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(animationButton);

		UITextButton* sponzaButton = new UITextButton(std::string("Sponza Scene"), glm::vec2((SCR_WIDTH / 6.0f) - 145.0f, SCR_HEIGHT * 0.65f), glm::vec2(0.4f, 0.4f), glm::vec2(400.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		sponzaButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		sponzaButton->SetMouseUpCallback(std::bind(&MainMenu::SponzaSceneButtonRelease, this, std::placeholders::_1));
		sponzaButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		sponzaButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(sponzaButton);

		UITextButton* aiButton = new UITextButton(std::string("AI Scene"), glm::vec2((SCR_WIDTH / 6.0f) - 145.0f, SCR_HEIGHT * 0.55f), glm::vec2(0.4f, 0.4f), glm::vec2(250.0f, 50.0f), font, glm::vec3(0.8f), 0);
		aiButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		aiButton->SetMouseUpCallback(std::bind(&MainMenu::AISceneButtonRelease, this, std::placeholders::_1));
		aiButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		aiButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(aiButton);

		UITextButton* iblButton = new UITextButton(std::string("IBL Scene"), glm::vec2((SCR_WIDTH / 6.0f) - 145.0f, SCR_HEIGHT * 0.45f), glm::vec2(0.4f, 0.4f), glm::vec2(260.0f, 50.0f), font, glm::vec3(0.8f), 0);
		iblButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		iblButton->SetMouseUpCallback(std::bind(&MainMenu::IBLSceneButtonRelease, this, std::placeholders::_1));
		iblButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		iblButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(iblButton);

		UITextButton* cullingButton = new UITextButton(std::string("Culling Scene"), glm::vec2((SCR_WIDTH / 6.0f) - 145.0f, SCR_HEIGHT * 0.35f), glm::vec2(0.4f, 0.4f), glm::vec2(380.0f, 50.0f), font, glm::vec3(0.8f), 0);
		cullingButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		cullingButton->SetMouseUpCallback(std::bind(&MainMenu::GeoCullingSceneButtonRelease, this, std::placeholders::_1));
		cullingButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		cullingButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(cullingButton);

		UITextButton* ssrButton = new UITextButton(std::string("SSR Scene"), glm::vec2((SCR_WIDTH / 6.0f) - 145.0f, SCR_HEIGHT * 0.25f), glm::vec2(0.4f, 0.4f), glm::vec2(310.0f, 50.0f), font, glm::vec3(0.8f), 0);
		ssrButton->SetMouseDownCallback(std::bind(&MainMenu::ButtonPress, this, std::placeholders::_1));
		ssrButton->SetMouseUpCallback(std::bind(&MainMenu::SSRSceneButtonRelease, this, std::placeholders::_1));
		ssrButton->SetMouseEnterCallback(std::bind(&MainMenu::ButtonEnter, this, std::placeholders::_1));
		ssrButton->SetMouseExitCallback(std::bind(&MainMenu::ButtonExit, this, std::placeholders::_1));
		canvas->AddUIElement(ssrButton);
	}

	void MainMenu::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}