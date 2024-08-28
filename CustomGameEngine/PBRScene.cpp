#include "PBRScene.h"
#include "GameInputManager.h"
#include "SystemPhysics.h"
#include "SystemUIRender.h"
#include "SystemUIMouseInteraction.h"
#include "UIText.h"
#include "UIImage.h"
#include "UITextButton.h"
#include "UIImageButton.h"
#include "SystemFrustumCulling.h"
#include <iomanip>
#include <sstream>
namespace Engine {
	PBRScene::PBRScene(SceneManager* sceneManager) : Scene(sceneManager, "PBRScene")
	{
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(400.0f);
		renderManager->GetRenderParams()->SetBloomPasses(10);
		renderManager->GetRenderParams()->SetSSAOSamples(32);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);

		ResourceManager::GetInstance()->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");

		//systemManager->BakeReflectionProbes(entityManager->Entities());
		renderManager->GetBakedData().LoadReflectionProbesFromFile();
	}

	PBRScene::~PBRScene()
	{

	}

	void PBRScene::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();

		ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/st_peters_square_night.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/newport_loft.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/metro_noord.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/laufenurg_church.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/golden_bay.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/metro_vijzelgracht.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/starsky.hdr", true);
		//ResourceManager::GetInstance()->LoadHDREnvironmentMap("Textures/Environment Maps/studio.hdr", true);

		envMaps.push_back("Textures/Environment Maps/st_peters_square_night.hdr");
		//envMaps.push_back("Textures/Environment Maps/newport_loft.hdr");
		//envMaps.push_back("Textures/Environment Maps/sky.hdr");
		//envMaps.push_back("Textures/Environment Maps/metro_noord.hdr");
		//envMaps.push_back("Textures/Environment Maps/laufenurg_church.hdr");
		//envMaps.push_back("Textures/Environment Maps/golden_bay.hdr");
		//envMaps.push_back("Textures/Environment Maps/metro_vijzelgracht.hdr");
		//envMaps.push_back("Textures/Environment Maps/starsky.hdr");
		//envMaps.push_back("Textures/Environment Maps/studio.hdr");
		currentEnvMapIndex = 0;

		renderManager->SetEnvironmentMap(envMaps[0]);
		renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);
	}

	
	void ButtonEnter(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(1.0f, 1.0f, 1.0f));
	}

	void ButtonExit(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));
	}

	void ButtonPress(UIButton* button) 
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.15f, 0.7f, 0.15f));
	}

	void BloomBtnRelease(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		RenderParams* renderParams = RenderManager::GetInstance()->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();

		bool bloom = (renderOptions & RENDER_BLOOM) != 0;
		bool advancedBloom = (renderOptions & RENDER_ADVANCED_BLOOM) != 0;
		bool lensDirt = (renderOptions & RENDER_ADVANCED_BLOOM_LENS_DIRT) != 0;

		if (!bloom && !advancedBloom) {
			renderParams->EnableRenderOptions(RENDER_BLOOM);
			std::cout << "PBRSCENE::ENABLE::Bloom" << std::endl;
			textButton->SetText("Bloom: on");
			textButton->SetButtonScale(glm::vec2(260.0f, 50.0f));
		}
		else if (bloom && !advancedBloom) {
			renderParams->DisableRenderOptions(RENDER_BLOOM);
			renderParams->EnableRenderOptions(RENDER_ADVANCED_BLOOM);
			std::cout << "PBRSCENE::DISABLE::Bloom" << std::endl;
			std::cout << "PBRSCENE::ENABLE::Adv Bloom" << std::endl;
			textButton->SetText("Bloom: adv");
			textButton->SetButtonScale(glm::vec2(280.0f, 50.0f));
		}
		else if (advancedBloom && !lensDirt) {
			renderParams->EnableRenderOptions(RENDER_ADVANCED_BLOOM_LENS_DIRT);
			std::cout << "PBRSCENE::ENABLE::Adv Bloom Lens Dirt" << std::endl;
			textButton->SetText("Bloom: adv + dirt");
			textButton->SetButtonScale(glm::vec2(445.0f, 50.0f));
		}
		else if (advancedBloom && lensDirt) {
			renderParams->DisableRenderOptions(RENDER_BLOOM | RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);
			std::cout << "PBRSCENE::DISABLE::Bloom" << std::endl;
			std::cout << "PBRSCENE::DISABLE::Adv Bloom" << std::endl;
			std::cout << "PBRSCENE::DISABLE::Adv Bloom Lens Dirt" << std::endl;
			textButton->SetText("Bloom: off");
			textButton->SetButtonScale(glm::vec2(260.0f, 50.0f));
		}
	}

	void SSAOBtnRelease(UIButton* button) {
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		RenderParams* renderParams = RenderManager::GetInstance()->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();

		if ((renderOptions & RENDER_SSAO) != 0) {
			renderParams->DisableRenderOptions(RENDER_SSAO);
			std::cout << "PBRSCENE::Disable::SSAO" << std::endl;
			textButton->SetText("SSAO: off");
		}
		else {
			renderParams->EnableRenderOptions(RENDER_SSAO);
			std::cout << "PBRSCENE::Enable::SSAO" << std::endl;
			textButton->SetText("SSAO: on");
		}
	}

	void ShadowsBtnRelease(UIButton* button) {
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		RenderParams* renderParams = RenderManager::GetInstance()->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();

		if ((renderOptions & RENDER_SHADOWS) != 0) {
			renderParams->DisableRenderOptions(RENDER_SHADOWS);
			std::cout << "PBRSCENE::Disable::Shadows" << std::endl;
			textButton->SetText("Shadows: off");
		}
		else {
			renderParams->EnableRenderOptions(RENDER_SHADOWS);
			std::cout << "PBRSCENE::Enable::Shadows" << std::endl;
			textButton->SetText("Shadows: on");
		}
	}

	void IBLBtnRelease(UIButton* button) {
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		RenderParams* renderParams = RenderManager::GetInstance()->GetRenderParams();
		RenderOptions renderOptions = renderParams->GetRenderOptions();

		if ((renderOptions & RENDER_IBL) != 0) {
			renderParams->DisableRenderOptions(RENDER_IBL);
			std::cout << "PBRSCENE::Disable::IBL" << std::endl;
			textButton->SetText("IBL: off");
		}
		else {
			renderParams->EnableRenderOptions(RENDER_IBL);
			std::cout << "PBRSCENE::Enable::IBL" << std::endl;
			textButton->SetText("IBL: on");
		}
	}

	void PBRScene::EnvMapBtnRelease(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		currentEnvMapIndex += 1;
		if (currentEnvMapIndex >= envMaps.size()) {
			currentEnvMapIndex = 0;
		}

		RenderManager::GetInstance()->SetEnvironmentMap(envMaps[currentEnvMapIndex]);

		std::cout << "PBRSCENE::ToggleEnvMap::" << currentEnvMapIndex << std::endl;
	}

	void PBRScene::ParameterGroupRelease(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.8f, 0.8f, 0.8f));

		for (UIElement* ui : parameterGroups[parameterGroupIndex]) {
			ui->SetActive(!ui->GetActive());
		}

		parameterGroupIndex += 1;
		if (parameterGroupIndex >= parameterGroups.size()) {
			parameterGroupIndex = 0;
		}

		for (UIElement* ui : parameterGroups[parameterGroupIndex]) {
			ui->SetActive(!ui->GetActive());
		}
	}

	void PBRScene::ParameterIncreaseOptionHold(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.15f, 0.7f, 0.15f));

		RenderParams* params = renderManager->GetRenderParams();
		bool shift = (inputManager->IsKeyDown(GLFW_KEY_LEFT_SHIFT) || inputManager->IsKeyDown(GLFW_KEY_RIGHT_SHIFT));

		std::ostringstream oss;
		std::string newText;

		float increase = 0.001f;
		if (shift) { increase = 0.01f; }

		int buttonId = button->GetIDTag();
		switch (parameterGroupIndex) {
		case 0:
			switch (buttonId) {
			case 1:
				// exposure
				params->SetExposure(params->GetExposure() + increase);

				oss = std::ostringstream();
				oss << "Exposure: " << std::setprecision(4) << renderManager->GetRenderParams()->GetExposure();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[0][1])->SetText(newText);
				break;
			case 2:
				// gamma
				params->SetGamma(params->GetGamma() + increase);

				oss = std::ostringstream();
				oss << "Gamma: " << std::setprecision(4) << renderManager->GetRenderParams()->GetGamma();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[0][4])->SetText(newText);
				break;
			}
			break;
		case 1:
			switch (buttonId) {
			case 1:
				increase *= 10.0f;
				params->SetAdvBloomThreshold(params->GetAdvBloomThreshold() + increase);

				oss = std::ostringstream();
				oss << "Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomThreshold();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][1])->SetText(newText);
				break;
			case 2:
				increase *= 10.0f;
				params->SetAdvBloomSoftThreshold(params->GetAdvBloomSoftThreshold() + increase);

				oss = std::ostringstream();
				oss << "Soft Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomSoftThreshold();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][4])->SetText(newText);
				break;
			case 3:
				params->SetAdvBloomFilterRadius(params->GetAdvBloomFilterRadius() + increase);

				oss = std::ostringstream();
				oss << "Filter Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomFilterRadius();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][7])->SetText(newText);
				break;
			case 4:
				params->SetAdvBloomStrength(params->GetAdvBloomStrength() + increase);

				oss = std::ostringstream();
				oss << "Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomStrength();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][10])->SetText(newText);
				break;
			case 5:
				params->SetAdvBloomLensDirtMaskStrength(params->GetAdvBloomLensDirtMaskStrength() + increase);

				oss = std::ostringstream();
				oss << "Dirt Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomLensDirtMaskStrength();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][13])->SetText(newText);
				break;
			}
			break;
		case 2:
			switch (buttonId) {
			case 1:
				increase = 1.0f;
				params->SetSSAOSamples(params->GetSSAOSamples() + (int)increase);

				newText = "Samples: " + std::to_string(params->GetSSAOSamples());

				dynamic_cast<UIText*>(parameterGroups[2][1])->SetText(newText);
				break;
			case 2:
				params->SetSSAORadius(params->GetSSAORadius() + increase);

				oss = std::ostringstream();
				oss << "Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAORadius();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[2][4])->SetText(newText);
				break;
			case 3:
				params->SetSSAOBias(params->GetSSAOBias() + increase);

				oss = std::ostringstream();
				oss << "Bias: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAOBias();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[2][7])->SetText(newText);
				break;
			}
			break;
		case 3:
			increase *= 5.0f;
			switch (buttonId) {
			case 1:
				directionalLight->Colour.r += increase;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Colour R: " << std::setprecision(4) << directionalLight->Colour.r;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][1])->SetText(newText);
				break;
			case 2:
				directionalLight->Colour.g += increase;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Colour G: " << std::setprecision(4) << directionalLight->Colour.g;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][4])->SetText(newText);
				break;
			case 3:
				directionalLight->Colour.b += increase;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Colour B: " << std::setprecision(4) << directionalLight->Colour.b;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][7])->SetText(newText);
				break;
			case 4:
				ambientStrength += increase;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Ambient strength: " << std::setprecision(4) << ambientStrength;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][10])->SetText(newText);
				break;
			case 5:
				increase *= 0.01f;
				directionalLight->MinShadowBias += increase;

				oss = std::ostringstream();
				oss << "Min Shadow Bias: " << std::setprecision(4) << directionalLight->MinShadowBias;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][13])->SetText(newText);
				break;
			case 6:
				increase *= 0.01f;
				directionalLight->MaxShadowBias += increase;

				oss = std::ostringstream();
				oss << "Max Shadow Bias: " << std::setprecision(4) << directionalLight->MaxShadowBias;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][16])->SetText(newText);
				break;
			}
			break;
		case 4:
			SystemRender * renderSystem = dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS));
			unsigned int currentEffect;
			switch (buttonId) {
			case 1:
				ChangePostProcessEffect();

				currentEffect = renderSystem->GetPostProcess();
				textButton->SetText(postProcessEffectToString[currentEffect]);
				break;
			case 2:
				increase *= 0.5f;
				params->SetPostProcessStrength(params->GetPostProcessStrength() + increase);

				oss = std::ostringstream();
				oss << "Post Process Strength: " << std::setprecision(4) << params->GetPostProcessStrength();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[4][2])->SetText(newText);
				break;
			}
			break;
		}
	}

	void PBRScene::ParameterDecreaseOptionHold(UIButton* button)
	{
		UITextButton* textButton = dynamic_cast<UITextButton*>(button);
		textButton->SetColour(glm::vec3(0.15f, 0.7f, 0.15f));

		RenderParams* params = renderManager->GetRenderParams();
		bool shift = (inputManager->IsKeyDown(GLFW_KEY_LEFT_SHIFT) || inputManager->IsKeyDown(GLFW_KEY_RIGHT_SHIFT));

		std::ostringstream oss;
		std::string newText;

		float decrease = 0.001f;
		if (shift) { decrease = 0.01f; }

		int buttonId = button->GetIDTag();
		switch (parameterGroupIndex) {
		case 0:
			switch (buttonId) {
			case 1:
				// exposure
				params->SetExposure(params->GetExposure() - decrease);

				oss = std::ostringstream();
				oss << "Exposure: " << std::setprecision(4) << renderManager->GetRenderParams()->GetExposure();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[0][1])->SetText(newText);
				break;
			case 2:
				// gamma
				params->SetGamma(params->GetGamma() - decrease);

				oss = std::ostringstream();
				oss << "Gamma: " << std::setprecision(4) << renderManager->GetRenderParams()->GetGamma();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[0][4])->SetText(newText);
				break;
			}
			break;
		case 1:
			switch (buttonId) {
			case 1:
				decrease *= 10.0f;
				params->SetAdvBloomThreshold(params->GetAdvBloomThreshold() - decrease);

				oss = std::ostringstream();
				oss << "Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomThreshold();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][1])->SetText(newText);
				break;
			case 2:
				decrease *= 10.0f;
				params->SetAdvBloomSoftThreshold(params->GetAdvBloomSoftThreshold() - decrease);

				oss = std::ostringstream();
				oss << "Soft Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomSoftThreshold();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][4])->SetText(newText);
				break;
			case 3:
				params->SetAdvBloomFilterRadius(params->GetAdvBloomFilterRadius() - decrease);

				oss = std::ostringstream();
				oss << "Filter Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomFilterRadius();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][7])->SetText(newText);
				break;
			case 4:
				params->SetAdvBloomStrength(params->GetAdvBloomStrength() - decrease);

				oss = std::ostringstream();
				oss << "Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomStrength();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][10])->SetText(newText);
				break;
			case 5:
				params->SetAdvBloomLensDirtMaskStrength(params->GetAdvBloomLensDirtMaskStrength() - decrease);

				oss = std::ostringstream();
				oss << "Dirt Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomLensDirtMaskStrength();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[1][13])->SetText(newText);
				break;
			}
			break;
		case 2:
			switch (buttonId) {
			case 1:
				decrease = 1.0f;
				params->SetSSAOSamples(params->GetSSAOSamples() - (int)decrease);

				newText = "Samples: " + std::to_string(params->GetSSAOSamples());

				dynamic_cast<UIText*>(parameterGroups[2][1])->SetText(newText);
				break;
			case 2:
				params->SetSSAORadius(params->GetSSAORadius() - decrease);

				oss = std::ostringstream();
				oss << "Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAORadius();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[2][4])->SetText(newText);
				break;
			case 3:
				params->SetSSAOBias(params->GetSSAOBias() - decrease);

				oss = std::ostringstream();
				oss << "Bias: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAOBias();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[2][7])->SetText(newText);
				break;
			}
		break;
		case 3:
			decrease *= 5.0f;
			switch (buttonId) {
			case 1:
				directionalLight->Colour.r -= decrease;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Colour R: " << std::setprecision(4) << directionalLight->Colour.r;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][1])->SetText(newText);
				break;
			case 2:
				directionalLight->Colour.g -= decrease;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Colour G: " << std::setprecision(4) << directionalLight->Colour.g;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][4])->SetText(newText);
				break;
			case 3:
				directionalLight->Colour.b -= decrease;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Colour B: " << std::setprecision(4) << directionalLight->Colour.b;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][7])->SetText(newText);
				break;
			case 4:
				ambientStrength -= decrease;
				directionalLight->Ambient = directionalLight->Colour * ambientStrength;

				oss = std::ostringstream();
				oss << "Ambient strength: " << std::setprecision(4) << ambientStrength;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][10])->SetText(newText);
				break;
			case 5:
				decrease *= 0.01f;
				directionalLight->MinShadowBias -= decrease;

				oss = std::ostringstream();
				oss << "Min Shadow Bias: " << std::setprecision(4) << directionalLight->MinShadowBias;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][13])->SetText(newText);
				break;
			case 6:
				decrease *= 0.01f;
				directionalLight->MaxShadowBias -= decrease;

				oss = std::ostringstream();
				oss << "Max Shadow Bias: " << std::setprecision(4) << directionalLight->MaxShadowBias;
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[3][16])->SetText(newText);
				break;
			}
			break;
		case 4:
			switch (buttonId) {
			case 1:
				break;
			case 2:
				decrease *= 0.5f;
				params->SetPostProcessStrength(params->GetPostProcessStrength() - decrease);

				oss = std::ostringstream();
				oss << "Post Process Strength: " << std::setprecision(4) << params->GetPostProcessStrength();
				newText = oss.str();

				dynamic_cast<UIText*>(parameterGroups[4][2])->SetText(newText);
				break;
			}
			break;
		}
	}

	void PBRScene::CreateEntities()
	{
		ambientStrength = 0.08f;

		Entity* dirLight = new Entity("Directional Light");
		dirLight->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		ComponentLight* directional = new ComponentLight(DIRECTIONAL);
		directional->CastShadows = true;
		//directional->Ambient = glm::vec3(0.01f, 0.01f, 0.05f);
		//directional->Ambient = glm::vec3(0.035f, 0.035f, 0.08f);
		directional->Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		directional->Ambient = directional->Colour * ambientStrength;
		//directional->Colour = glm::vec3(0.0f);
		directional->Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional->MinShadowBias = 0.0f;
		directional->MaxShadowBias = 0.003f;
		directional->DirectionalLightDistance = 20.0f;
		dirLight->AddComponent(directional);
		entityManager->AddEntity(dirLight);

		directionalLight = dirLight->GetLightComponent();

#pragma region materials
		PBRMaterial* bloomTest = new PBRMaterial();
		bloomTest->albedo = glm::vec3(1500.0f);
		bloomTest->metallic = 0.0f;
		bloomTest->roughness = 0.0f;
		bloomTest->ao = 0.0f;

		PBRMaterial* mirror = new PBRMaterial();
		mirror->albedo = glm::vec3(0.0f);
		mirror->metallic = 0.1f;
		mirror->roughness = 0.0f;
		mirror->ao = 1.0f;

		PBRMaterial* gold = new PBRMaterial();
		gold->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
		gold->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
		gold->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
		gold->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
		gold->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));

		PBRMaterial* rusted_iron = new PBRMaterial();
		rusted_iron->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/albedo.png", TEXTURE_ALBEDO, true));
		rusted_iron->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/normal.png", TEXTURE_NORMAL, false));
		rusted_iron->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/metallic.png", TEXTURE_METALLIC, false));
		rusted_iron->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/roughness.png", TEXTURE_ROUGHNESS, false));
		rusted_iron->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rusted_iron/ao.png", TEXTURE_AO, false));

		//PBRMaterial* plastic = new PBRMaterial();
		//plastic->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/albedo.png", TEXTURE_ALBEDO, true));
		//plastic->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/normal.png", TEXTURE_NORMAL, false));
		//plastic->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/metallic.png", TEXTURE_METALLIC, false));
		//plastic->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/roughness.png", TEXTURE_ROUGHNESS, false));
		//plastic->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/plastic/ao.png", TEXTURE_AO, false));

		PBRMaterial* bricks = new PBRMaterial();
		bricks->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
		bricks->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
		bricks->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
		bricks->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
		bricks->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
		bricks->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
		bricks->height_scale = -0.1;
		bricks->textureScaling = glm::vec2(10.0f);

		//PBRMaterial* grass = new PBRMaterial();
		//grass->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO, true));
		//grass->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL, false));
		//grass->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC, false));
		//grass->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS, false));
		//grass->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO, false));

		//PBRMaterial* leather = new PBRMaterial();
		//leather->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/albedo.png", TEXTURE_ALBEDO, true));
		//leather->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/normal.png", TEXTURE_NORMAL, false));
		//leather->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/leather/roughness.png", TEXTURE_ROUGHNESS, false));

		//PBRMaterial* metal_plate = new PBRMaterial();
		//metal_plate->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/albedo.png", TEXTURE_ALBEDO, true));
		//metal_plate->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/normal.png", TEXTURE_NORMAL, false));
		//metal_plate->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/metallic.png", TEXTURE_METALLIC, false));
		//metal_plate->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/roughness.png", TEXTURE_ROUGHNESS, false));
		//metal_plate->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/ao.png", TEXTURE_AO, false));
		//metal_plate->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/metal_plate/displacement.png", TEXTURE_DISPLACE, false));
		//metal_plate->height_scale = -0.1;

		PBRMaterial* scifi = new PBRMaterial();
		scifi->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/albedo.png", TEXTURE_ALBEDO, true));
		scifi->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/normal.png", TEXTURE_NORMAL, false));
		scifi->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/metallic.png", TEXTURE_METALLIC, false));
		scifi->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/roughness.png", TEXTURE_ROUGHNESS, false));
		scifi->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/ao.png", TEXTURE_AO, false));
		//scifi->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/scifi/displacement.png", TEXTURE_DISPLACE));
		//scifi->height_scale = -0.1;

		PBRMaterial* snow = new PBRMaterial();
		snow->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/albedo.png", TEXTURE_ALBEDO, true));
		snow->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/normal.png", TEXTURE_NORMAL, false));
		snow->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/specular.png", TEXTURE_METALLIC, false));
		snow->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/roughness.png", TEXTURE_ROUGHNESS, false));
		snow->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/ao.png", TEXTURE_AO, false));
		snow->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/snow/displacement.png", TEXTURE_DISPLACE, false));
		snow->height_scale = -0.1;

		PBRMaterial* space_blanket = new PBRMaterial();
		space_blanket->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/albedo.png", TEXTURE_ALBEDO, true));
		space_blanket->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/normal.png", TEXTURE_NORMAL, false));
		space_blanket->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/metallic.png", TEXTURE_METALLIC, false));
		space_blanket->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/roughness.png", TEXTURE_ROUGHNESS, false));
		space_blanket->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/space_blanket/ao.png", TEXTURE_AO, false));

		PBRMaterial* wall = new PBRMaterial();
		wall->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/albedo.png", TEXTURE_ALBEDO, false));
		wall->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/normal.png", TEXTURE_NORMAL, false));
		wall->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/metallic.png", TEXTURE_METALLIC, false));
		wall->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/roughness.png", TEXTURE_ROUGHNESS, false));
		wall->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/wall/ao.png", TEXTURE_AO, false));

		//PBRMaterial* worn_corrugated_iron = new PBRMaterial();
		//worn_corrugated_iron->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/albedo.png", TEXTURE_ALBEDO, true));
		//worn_corrugated_iron->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/normal.png", TEXTURE_NORMAL, false));
		//worn_corrugated_iron->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/roughness.png", TEXTURE_ROUGHNESS, false));
		//worn_corrugated_iron->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/ao.png", TEXTURE_AO, false));
		//worn_corrugated_iron->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/worn_corrugated_iron/displacement.png", TEXTURE_DISPLACE, false));
		//worn_corrugated_iron->height_scale = -0.1;

		//PBRMaterial* earth = new PBRMaterial();
		//earth->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/earth/albedo.jpg", TEXTURE_ALBEDO, true));
		//earth->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/earth/displacement.jpg", TEXTURE_DISPLACE, false));
		//earth->metallicMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/earth/specular.jpg", TEXTURE_METALLIC, false));
		//earth->height_scale = -0.1;

		PBRMaterial* raindrops = new PBRMaterial();
		raindrops->albedoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/albedo.jpg", TEXTURE_ALBEDO, true));
		raindrops->normalMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/normal.png", TEXTURE_NORMAL, false));
		raindrops->roughnessMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/roughness.jpg", TEXTURE_ROUGHNESS, false));
		raindrops->aoMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/ao.jpg", TEXTURE_AO, false));
		raindrops->heightMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/height.png", TEXTURE_DISPLACE, false));
		raindrops->opacityMaps.push_back(ResourceManager::GetInstance()->LoadTexture("Materials/PBR/rain_drops/opacity.png", TEXTURE_OPACITY, false));
		raindrops->isTransparent = true;
		raindrops->shadowCastAlphaDiscardThreshold = 1.0f;
		raindrops->textureScaling = glm::vec2(10.0f);
#pragma endregion

#pragma region scene
		Entity* floor = new Entity("Floor");
		floor->AddComponent(new ComponentTransform(0.0f, -1.0f, 0.0));
		floor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bricks);
		dynamic_cast<ComponentGeometry*>(floor->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(10.0f);
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(floor->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(floor);

		Entity* ceiling = new Entity("Cieling");
		ceiling->AddComponent(new ComponentTransform(0.0f, 5.0f, 0.0));
		ceiling->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(ceiling->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bricks);
		dynamic_cast<ComponentGeometry*>(ceiling->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(10.0f);
		dynamic_cast<ComponentTransform*>(ceiling->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(ceiling->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(1.0, 0.0, 0.0), 90.0f);
		entityManager->AddEntity(ceiling);

		Entity* rainFloor = new Entity("Rain Floor");
		rainFloor->AddComponent(new ComponentTransform(0.0f, -0.99f, 0.0f));
		rainFloor->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		rainFloor->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(raindrops);
		rainFloor->GetGeometryComponent()->SetTextureScale(10.0f);
		rainFloor->GetTransformComponent()->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
		rainFloor->GetTransformComponent()->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);
		entityManager->AddEntity(rainFloor);

		Entity* wall1 = new Entity("Wall 1");
		wall1->AddComponent(new ComponentTransform(0.0f, 0.0f, 10.0f));
		wall1->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(snow);
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall1->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall1->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 180.0f);
		entityManager->AddEntity(wall1);

		Entity* wall2 = new Entity("Wall 2");
		wall2->AddComponent(new ComponentTransform(0.0f, 0.0f, -10.0f));
		wall2->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(space_blanket);
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall2->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		entityManager->AddEntity(wall2);

		Entity* wall3 = new Entity("Wall 3");
		wall3->AddComponent(new ComponentTransform(10.0f, 0.0f, 0.0f));
		wall3->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall3->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall3->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), -90.0f);
		entityManager->AddEntity(wall3);

		Entity* wall4 = new Entity("Wall 4");
		wall4->AddComponent(new ComponentTransform(-10.0f, 0.0f, 0.0f));
		wall4->AddComponent(new ComponentGeometry(MODEL_PLANE, true));
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(scifi);
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->SetTextureScale(2.0f);
		dynamic_cast<ComponentGeometry*>(wall4->GetComponent(COMPONENT_GEOMETRY))->SetCulling(false, GL_BACK);
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
		dynamic_cast<ComponentTransform*>(wall4->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0, 1.0, 0.0), 90.0f);
		entityManager->AddEntity(wall4);

		Entity* pointLight = new Entity("Point Light");
		pointLight->AddComponent(new ComponentTransform(6.5f, 4.0f, -6.5f));
		pointLight->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light = new ComponentLight(POINT);
		light->Colour = glm::vec3(50.0, 50.0, 50.0);
		//light->Colour = glm::vec3(1.0, 1.0, 1.0);
		light->CastShadows = true;
		light->Active = true;
		pointLight->AddComponent(light);
		entityManager->AddEntity(pointLight);

		Entity* pointLight2 = new Entity("Point Light2");
		pointLight2->AddComponent(new ComponentTransform(-8.5f, 4.0f, 8.5f));
		pointLight2->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight2->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight2->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light2 = new ComponentLight(POINT);
		light2->Colour = glm::vec3(150.0, 150.0, 150.0);
		//light2->Colour = glm::vec3(1.0, 1.0, 1.0);
		light2->CastShadows = true;
		light2->Active = true;
		pointLight2->AddComponent(light2);
		entityManager->AddEntity(pointLight2);

		Entity* pointLight3 = new Entity("Point Light3");
		pointLight3->AddComponent(new ComponentTransform(6.5f, 4.0f, 6.5f));
		pointLight3->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight3->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight3->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light3 = new ComponentLight(POINT);
		light3->Colour = glm::vec3(50.0, 50.0, 50.0);
		//light3->Colour = glm::vec3(1.0, 1.0, 1.0);
		light3->CastShadows = true;
		light3->Active = true;
		pointLight3->AddComponent(light3);
		entityManager->AddEntity(pointLight3);

		Entity* pointLight4 = new Entity("Point Light4");
		pointLight4->AddComponent(new ComponentTransform(-6.5f, 2.0f, -6.5f));
		pointLight4->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight4->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight4->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light4 = new ComponentLight(POINT);
		light4->Colour = glm::vec3(50.0, 50.0, 50.0);
		//light4->Colour = glm::vec3(1.0, 1.0, 1.0);
		light4->CastShadows = true;
		light4->Active = true;
		pointLight4->AddComponent(light4);
		entityManager->AddEntity(pointLight4);

		Entity* pointLight5 = new Entity("Point Light5");
		pointLight5->AddComponent(new ComponentTransform(0.0f, 2.0f, 0.0f));
		pointLight5->AddComponent(new ComponentGeometry(MODEL_SPHERE, true));
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(rusted_iron);
		dynamic_cast<ComponentGeometry*>(pointLight5->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(pointLight5->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		ComponentLight* light5 = new ComponentLight(POINT);
		light5->Colour = glm::vec3(25.0f, 25.0f, 25.0f);
		//light5->Colour = glm::vec3(0.5, 0.5, 0.5);
		light5->Linear = 0.027f;
		light5->Quadratic = 0.0028f;
		light5->CastShadows = true;
		light5->Active = true;
		pointLight5->AddComponent(light5);
		entityManager->AddEntity(pointLight5);

		Entity* spotParent = new Entity("Spot Parent");
		spotParent->AddComponent(new ComponentTransform(1.0f, 0.0f, 4.6f));
		spotParent->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(raindrops);
		dynamic_cast<ComponentGeometry*>(spotParent->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(spotParent->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(1.0f));
		entityManager->AddEntity(spotParent);

		Entity* spotLight = new Entity("Spot Light");
		spotLight->AddComponent(new ComponentTransform(-5.5f, 1.0f, 0.0f));
		spotLight->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(gold);
		dynamic_cast<ComponentGeometry*>(spotLight->GetComponent(COMPONENT_GEOMETRY))->CastShadows(true);
		dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.25f));
		//dynamic_cast<ComponentTransform*>(spotLight->GetComponent(COMPONENT_TRANSFORM))->SetParent(spotParent);
		ComponentLight* spot = new ComponentLight(SPOT);
		spot->Colour = glm::vec3(70.0f, 20.0f, 40.0f);
		//spot->Colour = glm::vec3(0.7f, 0.2f, 0.4f);
		spot->CastShadows = true;
		spot->Direction = glm::vec3(-1.0f, 0.0f, 0.0f);
		spot->Cutoff = glm::cos(glm::radians(20.0f));
		spot->OuterCutoff = glm::cos(glm::radians(32.0f));
		spotLight->AddComponent(spot);
		entityManager->AddEntity(spotLight);

		Entity* goblet = new Entity("Goblet");
		goblet->AddComponent(new ComponentTransform(0.0f, 0.35f, 0.0f));
		dynamic_cast<ComponentTransform*>(goblet->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(4.0f));
		goblet->AddComponent(new ComponentGeometry("Models/PBR/brass_goblet/brass_goblet.obj", true));
		entityManager->AddEntity(goblet);

		Entity* gobletClone = goblet->Clone();
		gobletClone->GetTransformComponent()->SetPosition(gobletClone->GetTransformComponent()->GetWorldPosition() + glm::vec3(0.0f, 5.0f, 0.0f));

		Entity* cart = new Entity("Cart");
		cart->AddComponent(new ComponentTransform(5.0f, -1.0f, 2.0f));
		dynamic_cast<ComponentTransform*>(cart->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(2.0f));
		cart->AddComponent(new ComponentGeometry("Models/PBR/cart/cart.obj", true));
		entityManager->AddEntity(cart);

		//Entity* bloomCube = new Entity("Bloom Cube");
		//bloomCube->AddComponent(new ComponentTransform(-2.5f, 0.35f, 2.5f));
		//dynamic_cast<ComponentTransform*>(bloomCube->GetComponent(COMPONENT_TRANSFORM))->SetScale(glm::vec3(0.5f));
		//bloomCube->AddComponent(new ComponentGeometry(MODEL_CUBE, true));
		//ComponentLight* bloomLight = new ComponentLight(POINT);
		////bloomLight->Colour = glm::vec3(50.0f, 50.0f, 50.0f);
		//bloomLight->Colour = glm::vec3(0.5f, 0.5f, 0.5f);
		//bloomLight->CastShadows = false;
		//bloomCube->AddComponent(bloomLight);
		//dynamic_cast<ComponentGeometry*>(bloomCube->GetComponent(COMPONENT_GEOMETRY))->GetModel()->ApplyMaterialToAllMesh(bloomTest);
		//entityManager->AddEntity(bloomCube);
#pragma endregion

#pragma region ui
		TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");

		Entity* canvas = new Entity("Canvas");
		canvas->AddComponent(new ComponentTransform(0.0f, 0.0f, 0.0f));
		canvas->GetTransformComponent()->SetScale(1.0f);
		canvas->AddComponent(new ComponentUICanvas(SCREEN_SPACE));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(25.0f, 135.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(25.0f, 10.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Resolution: ") + std::to_string(SCR_WIDTH) + " X " + std::to_string(SCR_HEIGHT), glm::vec2(25.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("Shadow res: ") + std::to_string(renderManager->ShadowWidth()) + " X " + std::to_string(renderManager->ShadowHeight()), glm::vec2(25.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		std::string renderPipeline = "DEFERRED";
		if (renderManager->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
			renderPipeline = "FORWARD";
		}
		canvas->GetUICanvasComponent()->AddUIElement(new UIText(std::string("G Pipeline: ") + renderPipeline, glm::vec2(25.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

		// Bloom button
		UITextButton* bloomBtn = new UITextButton(std::string("Bloom: adv + dirt"), glm::vec2(355.0f, 60.0f), glm::vec2(0.4f, 0.4f), glm::vec2(445.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
		bloomBtn->SetMouseEnterCallback(ButtonEnter);
		bloomBtn->SetMouseExitCallback(ButtonExit);
		bloomBtn->SetMouseDownCallback(ButtonPress);
		bloomBtn->SetMouseUpCallback(BloomBtnRelease);
		bloomBtn->SetActive(false);
		optionButtons.push_back(bloomBtn);
		canvas->GetUICanvasComponent()->AddUIElement(bloomBtn);

		// SSAO button
		UITextButton* ssaoBtn = new UITextButton(std::string("SSAO: on"), glm::vec2(850.0f, 60.0f), glm::vec2(0.4f), glm::vec2(260.0f, 50.0f), font, glm::vec3(0.8f), 0);
		ssaoBtn->SetMouseEnterCallback(ButtonEnter);
		ssaoBtn->SetMouseExitCallback(ButtonExit);
		ssaoBtn->SetMouseDownCallback(ButtonPress);
		ssaoBtn->SetMouseUpCallback(SSAOBtnRelease);
		ssaoBtn->SetActive(false);
		optionButtons.push_back(ssaoBtn);
		canvas->GetUICanvasComponent()->AddUIElement(ssaoBtn);

		// Shadows button
		UITextButton* shadowsBtn = new UITextButton(std::string("Shadows: on"), glm::vec2(1170.0f, 60.0f), glm::vec2(0.4f), glm::vec2(330.0f, 50.0f), font, glm::vec3(0.8f), 0);
		shadowsBtn->SetMouseEnterCallback(ButtonEnter);
		shadowsBtn->SetMouseExitCallback(ButtonExit);
		shadowsBtn->SetMouseDownCallback(ButtonPress);
		shadowsBtn->SetMouseUpCallback(ShadowsBtnRelease);
		shadowsBtn->SetActive(false);
		optionButtons.push_back(shadowsBtn);
		canvas->GetUICanvasComponent()->AddUIElement(shadowsBtn);

		// IBL button
		UITextButton* iblBtn = new UITextButton(std::string("IBL: on"), glm::vec2(1550.0f, 60.0f), glm::vec2(0.4f), glm::vec2(185.0f, 50.0f), font, glm::vec3(0.8f), 0);
		iblBtn->SetMouseEnterCallback(ButtonEnter);
		iblBtn->SetMouseExitCallback(ButtonExit);
		iblBtn->SetMouseDownCallback(ButtonPress);
		iblBtn->SetMouseUpCallback(IBLBtnRelease);
		iblBtn->SetActive(false);
		optionButtons.push_back(iblBtn);
		canvas->GetUICanvasComponent()->AddUIElement(iblBtn);

		// Env map button
		UITextButton* envMapBtn = new UITextButton(std::string("Switch Env Map"), glm::vec2(1900.0f, 60.0f), glm::vec2(0.4f), glm::vec2(420.0f, 50.0f), font, glm::vec3(0.8f), 0);
		envMapBtn->SetMouseEnterCallback(ButtonEnter);
		envMapBtn->SetMouseExitCallback(ButtonExit);
		envMapBtn->SetMouseDownCallback(ButtonPress);
		envMapBtn->SetMouseUpCallback(std::bind(&PBRScene::EnvMapBtnRelease, this, std::placeholders::_1));
		envMapBtn->SetActive(false);
		optionButtons.push_back(envMapBtn);
		canvas->GetUICanvasComponent()->AddUIElement(envMapBtn);

		// Render param btn groups
		// Tonemapping group
		std::vector<UIElement*> group;

		std::ostringstream oss;

		UIBackground tonemappingBackground;
		tonemappingBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		tonemappingBackground.Bordered = true;
		tonemappingBackground.BorderThickness = 0.01f;
		tonemappingBackground.Colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.85f);
		tonemappingBackground.LeftRightUpDownExtents = glm::vec4(0.05f, 0.325f, 0.15f, 0.25f);

		UITextButton* tonemappingParams = new UITextButton(std::string("Tonemapping:"), glm::vec2(25.0f, (float)SCR_HEIGHT - 70.0f), glm::vec2(0.4f), glm::vec2(350.0f, 50.0f), font, glm::vec3(0.8f), 0, tonemappingBackground);
		tonemappingParams->SetMouseEnterCallback(ButtonEnter);
		tonemappingParams->SetMouseExitCallback(ButtonExit);
		tonemappingParams->SetMouseDownCallback(ButtonPress);
		tonemappingParams->SetMouseUpCallback(std::bind(&PBRScene::ParameterGroupRelease, this, std::placeholders::_1));
		tonemappingParams->SetActive(false);
		group.push_back(tonemappingParams);
		canvas->GetUICanvasComponent()->AddUIElement(tonemappingParams);

		// Exposure
		oss << "Exposure: " << std::setprecision(4) << renderManager->GetRenderParams()->GetExposure();
		std::string exposureString = oss.str();
		oss = std::ostringstream();
		UIText* exposure = new UIText(exposureString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		exposure->SetActive(false);
		group.push_back(exposure);
		canvas->GetUICanvasComponent()->AddUIElement(exposure);

		UITextButton* exposureIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		exposureIncrease->SetMouseEnterCallback(ButtonEnter);
		exposureIncrease->SetMouseExitCallback(ButtonExit);
		exposureIncrease->SetMouseUpCallback(ButtonEnter);
		exposureIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		exposureIncrease->SetActive(false);
		group.push_back(exposureIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(exposureIncrease);

		UITextButton* exposureDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		exposureDecrease->SetMouseEnterCallback(ButtonEnter);
		exposureDecrease->SetMouseExitCallback(ButtonExit);
		exposureDecrease->SetMouseUpCallback(ButtonEnter);
		exposureDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		exposureDecrease->SetActive(false);
		group.push_back(exposureDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(exposureDecrease);

		// Gamma
		UIText* gamma = new UIText(std::string("Gamma: " + std::to_string(renderManager->GetRenderParams()->GetGamma())), glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		gamma->SetActive(false);
		group.push_back(gamma);
		canvas->GetUICanvasComponent()->AddUIElement(gamma);

		UITextButton* gammaIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		gammaIncrease->SetMouseEnterCallback(ButtonEnter);
		gammaIncrease->SetMouseExitCallback(ButtonExit);
		gammaIncrease->SetMouseUpCallback(ButtonEnter);
		gammaIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		gammaIncrease->SetActive(false);
		group.push_back(gammaIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(gammaIncrease);

		UITextButton* gammaDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		gammaDecrease->SetMouseEnterCallback(ButtonEnter);
		gammaDecrease->SetMouseExitCallback(ButtonExit);
		gammaDecrease->SetMouseUpCallback(ButtonEnter);
		gammaDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		gammaDecrease->SetActive(false);
		group.push_back(gammaDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(gammaDecrease);

		parameterGroups.push_back(group);

		group.clear();

		// Advanced bloom (pbr bloom) group

		UIBackground advBloomBackground;
		advBloomBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		advBloomBackground.Bordered = true;
		advBloomBackground.BorderThickness = 0.01f;
		advBloomBackground.Colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.85f);
		advBloomBackground.LeftRightUpDownExtents = glm::vec4(0.05f, 0.375f, 0.15f, 0.4f);

		UITextButton* advBloomParams = new UITextButton(std::string("Advanced Bloom:"), glm::vec2(25.0f, (float)SCR_HEIGHT - 70.0f), glm::vec2(0.4f), glm::vec2(380.0f, 50.0f), font, glm::vec3(0.8f), 0, advBloomBackground);
		advBloomParams->SetMouseEnterCallback(ButtonEnter);
		advBloomParams->SetMouseExitCallback(ButtonExit);
		advBloomParams->SetMouseDownCallback(ButtonPress);
		advBloomParams->SetMouseUpCallback(std::bind(&PBRScene::ParameterGroupRelease, this, std::placeholders::_1));
		advBloomParams->SetActive(false);
		group.push_back(advBloomParams);
		canvas->GetUICanvasComponent()->AddUIElement(advBloomParams);

		// Threshold
		oss << "Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomThreshold();
		std::string thresholdString = oss.str();
		oss = std::ostringstream();
		UIText* threshold = new UIText(thresholdString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		threshold->SetActive(false);
		group.push_back(threshold);
		canvas->GetUICanvasComponent()->AddUIElement(threshold);

		UITextButton* thresholdIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		thresholdIncrease->SetMouseEnterCallback(ButtonEnter);
		thresholdIncrease->SetMouseExitCallback(ButtonExit);
		thresholdIncrease->SetMouseUpCallback(ButtonEnter);
		thresholdIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		thresholdIncrease->SetActive(false);
		group.push_back(thresholdIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(thresholdIncrease);

		UITextButton* thresholdDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		thresholdDecrease->SetMouseEnterCallback(ButtonEnter);
		thresholdDecrease->SetMouseExitCallback(ButtonExit);
		thresholdDecrease->SetMouseUpCallback(ButtonEnter);
		thresholdDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		thresholdDecrease->SetActive(false);
		group.push_back(thresholdDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(thresholdDecrease);

		// Soft threshold
		oss << "Soft Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomSoftThreshold();
		std::string softThresholdString = oss.str();
		oss = std::ostringstream();
		UIText* softThreshold = new UIText(softThresholdString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		softThreshold->SetActive(false);
		group.push_back(softThreshold);
		canvas->GetUICanvasComponent()->AddUIElement(softThreshold);

		UITextButton* softThresholdIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		softThresholdIncrease->SetMouseEnterCallback(ButtonEnter);
		softThresholdIncrease->SetMouseExitCallback(ButtonExit);
		softThresholdIncrease->SetMouseUpCallback(ButtonEnter);
		softThresholdIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		softThresholdIncrease->SetActive(false);
		group.push_back(softThresholdIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(softThresholdIncrease);

		UITextButton* softThresholdDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		softThresholdDecrease->SetMouseEnterCallback(ButtonEnter);
		softThresholdDecrease->SetMouseExitCallback(ButtonExit);
		softThresholdDecrease->SetMouseUpCallback(ButtonEnter);
		softThresholdDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		softThresholdDecrease->SetActive(false);
		group.push_back(softThresholdDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(softThresholdDecrease);

		// Filter radius
		oss << "Filter Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomFilterRadius();
		std::string filterRadiusString = oss.str();
		oss = std::ostringstream();
		UIText* filterRadius = new UIText(filterRadiusString, glm::vec2(60.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		filterRadius->SetActive(false);
		group.push_back(filterRadius);
		canvas->GetUICanvasComponent()->AddUIElement(filterRadius);

		UITextButton* filterRadiusIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
		filterRadiusIncrease->SetMouseEnterCallback(ButtonEnter);
		filterRadiusIncrease->SetMouseExitCallback(ButtonExit);
		filterRadiusIncrease->SetMouseUpCallback(ButtonEnter);
		filterRadiusIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		filterRadiusIncrease->SetActive(false);
		group.push_back(filterRadiusIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(filterRadiusIncrease);

		UITextButton* filterRadiusDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
		filterRadiusDecrease->SetMouseEnterCallback(ButtonEnter);
		filterRadiusDecrease->SetMouseExitCallback(ButtonExit);
		filterRadiusDecrease->SetMouseUpCallback(ButtonEnter);
		filterRadiusDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		filterRadiusDecrease->SetActive(false);
		group.push_back(filterRadiusDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(filterRadiusDecrease);

		// Strength
		oss << "Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomStrength();
		std::string strengthString = oss.str();
		oss = std::ostringstream();
		UIText* strength = new UIText(strengthString, glm::vec2(60.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		strength->SetActive(false);
		group.push_back(strength);
		canvas->GetUICanvasComponent()->AddUIElement(strength);

		UITextButton* strengthIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
		strengthIncrease->SetMouseEnterCallback(ButtonEnter);
		strengthIncrease->SetMouseExitCallback(ButtonExit);
		strengthIncrease->SetMouseUpCallback(ButtonEnter);
		strengthIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		strengthIncrease->SetActive(false);
		group.push_back(strengthIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(strengthIncrease);

		UITextButton* strengthDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
		strengthDecrease->SetMouseEnterCallback(ButtonEnter);
		strengthDecrease->SetMouseExitCallback(ButtonExit);
		strengthDecrease->SetMouseUpCallback(ButtonEnter);
		strengthDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		strengthDecrease->SetActive(false);
		group.push_back(strengthDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(strengthDecrease);

		// Dirt strength
		oss << "Dirt Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomLensDirtMaskStrength();
		std::string dirtStrengthString = oss.str();
		oss = std::ostringstream();
		UIText* dirtStrength = new UIText(dirtStrengthString, glm::vec2(60.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		dirtStrength->SetActive(false);
		group.push_back(dirtStrength);
		canvas->GetUICanvasComponent()->AddUIElement(dirtStrength);

		UITextButton* dirtStrengthIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
		dirtStrengthIncrease->SetMouseEnterCallback(ButtonEnter);
		dirtStrengthIncrease->SetMouseExitCallback(ButtonExit);
		dirtStrengthIncrease->SetMouseUpCallback(ButtonEnter);
		dirtStrengthIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		dirtStrengthIncrease->SetActive(false);
		group.push_back(dirtStrengthIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(dirtStrengthIncrease);

		UITextButton* dirtStrengthDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
		dirtStrengthDecrease->SetMouseEnterCallback(ButtonEnter);
		dirtStrengthDecrease->SetMouseExitCallback(ButtonExit);
		dirtStrengthDecrease->SetMouseUpCallback(ButtonEnter);
		dirtStrengthDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		dirtStrengthDecrease->SetActive(false);
		group.push_back(dirtStrengthDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(dirtStrengthDecrease);

		parameterGroups.push_back(group);
		group.clear();

		// SSAO params
		UIBackground ssaoBackground;
		ssaoBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		ssaoBackground.Bordered = true;
		ssaoBackground.BorderThickness = 0.01f;
		ssaoBackground.Colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.85f);
		ssaoBackground.LeftRightUpDownExtents = glm::vec4(0.05f, 0.4f, 0.15f, 0.3f);

		UITextButton* ssaoParams = new UITextButton(std::string("Ambient occlusion:"), glm::vec2(25.0f, (float)SCR_HEIGHT - 70.0f), glm::vec2(0.4f), glm::vec2(380.0f, 50.0f), font, glm::vec3(0.8f), 0, ssaoBackground);
		ssaoParams->SetMouseEnterCallback(ButtonEnter);
		ssaoParams->SetMouseExitCallback(ButtonExit);
		ssaoParams->SetMouseDownCallback(ButtonPress);
		ssaoParams->SetMouseUpCallback(std::bind(&PBRScene::ParameterGroupRelease, this, std::placeholders::_1));
		ssaoParams->SetActive(false);
		group.push_back(ssaoParams);
		canvas->GetUICanvasComponent()->AddUIElement(ssaoParams);

		// Samples
		UIText* samples = new UIText(std::string("Samples: " + std::to_string(renderManager->GetRenderParams()->GetSSAOSamples())), glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		samples->SetActive(false);
		group.push_back(samples);
		canvas->GetUICanvasComponent()->AddUIElement(samples);

		UITextButton* samplesIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		samplesIncrease->SetMouseEnterCallback(ButtonEnter);
		samplesIncrease->SetMouseExitCallback(ButtonExit);
		samplesIncrease->SetMouseUpCallback(ButtonEnter);
		samplesIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		samplesIncrease->SetActive(false);
		group.push_back(samplesIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(samplesIncrease);

		UITextButton* samplesDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		samplesDecrease->SetMouseEnterCallback(ButtonEnter);
		samplesDecrease->SetMouseExitCallback(ButtonExit);
		samplesDecrease->SetMouseUpCallback(ButtonEnter);
		samplesDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		samplesDecrease->SetActive(false);
		group.push_back(samplesDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(samplesDecrease);

		// Radius
		oss << "Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAOBias();
		std::string radiusString = oss.str();
		oss = std::ostringstream();
		UIText* radius = new UIText(radiusString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		radius->SetActive(false);
		group.push_back(radius);
		canvas->GetUICanvasComponent()->AddUIElement(radius);

		UITextButton* radiusIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		radiusIncrease->SetMouseEnterCallback(ButtonEnter);
		radiusIncrease->SetMouseExitCallback(ButtonExit);
		radiusIncrease->SetMouseUpCallback(ButtonEnter);
		radiusIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		radiusIncrease->SetActive(false);
		group.push_back(radiusIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(radiusIncrease);

		UITextButton* radiusDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		radiusDecrease->SetMouseEnterCallback(ButtonEnter);
		radiusDecrease->SetMouseExitCallback(ButtonExit);
		radiusDecrease->SetMouseUpCallback(ButtonEnter);
		radiusDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		radiusDecrease->SetActive(false);
		group.push_back(radiusDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(radiusDecrease);

		// Bias
		oss << "Bias: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAOBias();
		std::string biasString = oss.str();
		oss = std::ostringstream();
		UIText* bias = new UIText(biasString, glm::vec2(60.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		bias->SetActive(false);
		group.push_back(bias);
		canvas->GetUICanvasComponent()->AddUIElement(bias);

		UITextButton* biasIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
		biasIncrease->SetMouseEnterCallback(ButtonEnter);
		biasIncrease->SetMouseExitCallback(ButtonExit);
		biasIncrease->SetMouseUpCallback(ButtonEnter);
		biasIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		biasIncrease->SetActive(false);
		group.push_back(biasIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(biasIncrease);

		UITextButton* biasDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
		biasDecrease->SetMouseEnterCallback(ButtonEnter);
		biasDecrease->SetMouseExitCallback(ButtonExit);
		biasDecrease->SetMouseUpCallback(ButtonEnter);
		biasDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		biasDecrease->SetActive(false);
		group.push_back(biasDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(biasDecrease);

		parameterGroups.push_back(group);
		group.clear();

		// Directional light edit group
		UIBackground dirLightBackground;
		dirLightBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		dirLightBackground.Bordered = true;
		dirLightBackground.BorderThickness = 0.01f;
		dirLightBackground.Colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.85f);
		dirLightBackground.LeftRightUpDownExtents = glm::vec4(0.05f, 0.35f, 0.15f, 0.435f);

		UITextButton* dirLightParams = new UITextButton(std::string("Directional light:"), glm::vec2(25.0f, (float)SCR_HEIGHT - 70.0f), glm::vec2(0.4f), glm::vec2(380.0f, 50.0f), font, glm::vec3(0.8f), 0, dirLightBackground);
		dirLightParams->SetMouseEnterCallback(ButtonEnter);
		dirLightParams->SetMouseExitCallback(ButtonExit);
		dirLightParams->SetMouseDownCallback(ButtonPress);
		dirLightParams->SetMouseUpCallback(std::bind(&PBRScene::ParameterGroupRelease, this, std::placeholders::_1));
		dirLightParams->SetActive(false);
		group.push_back(dirLightParams);
		canvas->GetUICanvasComponent()->AddUIElement(dirLightParams);

		// Colour R
		oss << "Colour R: " << std::setprecision(3) << dirLight->GetLightComponent()->Colour.r;
		std::string cRString = oss.str();
		oss = std::ostringstream();
		UIText* cR = new UIText(cRString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		cR->SetActive(false);
		group.push_back(cR);
		canvas->GetUICanvasComponent()->AddUIElement(cR);

		UITextButton* rIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		rIncrease->SetMouseEnterCallback(ButtonEnter);
		rIncrease->SetMouseExitCallback(ButtonExit);
		rIncrease->SetMouseUpCallback(ButtonEnter);
		rIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		rIncrease->SetActive(false);
		group.push_back(rIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(rIncrease);

		UITextButton* rDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
		rDecrease->SetMouseEnterCallback(ButtonEnter);
		rDecrease->SetMouseExitCallback(ButtonExit);
		rDecrease->SetMouseUpCallback(ButtonEnter);
		rDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		rDecrease->SetActive(false);
		group.push_back(rDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(rDecrease);

		// Colour G
		oss << "Colour G: " << std::setprecision(3) << dirLight->GetLightComponent()->Colour.g;
		std::string cGString = oss.str();
		oss = std::ostringstream();
		UIText* cG = new UIText(cGString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		cG->SetActive(false);
		group.push_back(cG);
		canvas->GetUICanvasComponent()->AddUIElement(cG);

		UITextButton* gIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		gIncrease->SetMouseEnterCallback(ButtonEnter);
		gIncrease->SetMouseExitCallback(ButtonExit);
		gIncrease->SetMouseUpCallback(ButtonEnter);
		gIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		gIncrease->SetActive(false);
		group.push_back(gIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(gIncrease);

		UITextButton* gDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		gDecrease->SetMouseEnterCallback(ButtonEnter);
		gDecrease->SetMouseExitCallback(ButtonExit);
		gDecrease->SetMouseUpCallback(ButtonEnter);
		gDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		gDecrease->SetActive(false);
		group.push_back(gDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(gDecrease);

		// Colour B
		oss << "Colour B: " << std::setprecision(3) << dirLight->GetLightComponent()->Colour.b;
		std::string cBString = oss.str();
		oss = std::ostringstream();
		UIText* cB = new UIText(cBString, glm::vec2(60.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		cB->SetActive(false);
		group.push_back(cB);
		canvas->GetUICanvasComponent()->AddUIElement(cB);

		UITextButton* bIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
		bIncrease->SetMouseEnterCallback(ButtonEnter);
		bIncrease->SetMouseExitCallback(ButtonExit);
		bIncrease->SetMouseUpCallback(ButtonEnter);
		bIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		bIncrease->SetActive(false);
		group.push_back(bIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(bIncrease);

		UITextButton* bDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
		bDecrease->SetMouseEnterCallback(ButtonEnter);
		bDecrease->SetMouseExitCallback(ButtonExit);
		bDecrease->SetMouseUpCallback(ButtonEnter);
		bDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		bDecrease->SetActive(false);
		group.push_back(bDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(bDecrease);

		// Ambient strength
		oss << "Ambient strength: " << std::setprecision(3) << ambientStrength;
		std::string ambientString = oss.str();
		oss = std::ostringstream();
		UIText* ambientStrength = new UIText(ambientString, glm::vec2(60.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		ambientStrength->SetActive(false);
		group.push_back(ambientStrength);
		canvas->GetUICanvasComponent()->AddUIElement(ambientStrength);

		UITextButton* ambientIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
		ambientIncrease->SetMouseEnterCallback(ButtonEnter);
		ambientIncrease->SetMouseExitCallback(ButtonExit);
		ambientIncrease->SetMouseUpCallback(ButtonEnter);
		ambientIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		ambientIncrease->SetActive(false);
		group.push_back(ambientIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(ambientIncrease);

		UITextButton* ambientDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
		ambientDecrease->SetMouseEnterCallback(ButtonEnter);
		ambientDecrease->SetMouseExitCallback(ButtonExit);
		ambientDecrease->SetMouseUpCallback(ButtonEnter);
		ambientDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		ambientDecrease->SetActive(false);
		group.push_back(ambientDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(ambientDecrease);

		// Min Shadow Bias
		oss << "Min Shadow Bias: " << std::setprecision(4) << directionalLight->MinShadowBias;
		std::string minBiasString = oss.str();
		oss = std::ostringstream();
		UIText* minBias = new UIText(minBiasString, glm::vec2(60.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		minBias->SetActive(false);
		group.push_back(minBias);
		canvas->GetUICanvasComponent()->AddUIElement(minBias);

		UITextButton* minBiasIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
		minBiasIncrease->SetMouseEnterCallback(ButtonEnter);
		minBiasIncrease->SetMouseExitCallback(ButtonExit);
		minBiasIncrease->SetMouseUpCallback(ButtonEnter);
		minBiasIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		minBiasIncrease->SetActive(false);
		group.push_back(minBiasIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(minBiasIncrease);

		UITextButton* minBiasDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
		minBiasDecrease->SetMouseEnterCallback(ButtonEnter);
		minBiasDecrease->SetMouseExitCallback(ButtonExit);
		minBiasDecrease->SetMouseUpCallback(ButtonEnter);
		minBiasDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		minBiasDecrease->SetActive(false);
		group.push_back(minBiasDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(minBiasDecrease);

		// Max Shadow Bias
		oss << "Max Shadow Bias: " << std::setprecision(4) << directionalLight->MaxShadowBias;
		std::string maxBiasString = oss.str();
		oss = std::ostringstream();
		UIText* maxBias = new UIText(maxBiasString, glm::vec2(60.0f, (float)SCR_HEIGHT - 320.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		maxBias->SetActive(false);
		group.push_back(maxBias);
		canvas->GetUICanvasComponent()->AddUIElement(maxBias);

		UITextButton* maxBiasIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 320.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 6);
		maxBiasIncrease->SetMouseEnterCallback(ButtonEnter);
		maxBiasIncrease->SetMouseExitCallback(ButtonExit);
		maxBiasIncrease->SetMouseUpCallback(ButtonEnter);
		maxBiasIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		maxBiasIncrease->SetActive(false);
		group.push_back(maxBiasIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(maxBiasIncrease);

		UITextButton* maxBiasDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 320.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 6);
		maxBiasDecrease->SetMouseEnterCallback(ButtonEnter);
		maxBiasDecrease->SetMouseExitCallback(ButtonExit);
		maxBiasDecrease->SetMouseUpCallback(ButtonEnter);
		maxBiasDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		maxBiasDecrease->SetActive(false);
		group.push_back(maxBiasDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(maxBiasDecrease);

		parameterGroups.push_back(group);
		group.clear();

		// Post processing parameter group
		UIBackground postProcessingBackground;
		postProcessingBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		postProcessingBackground.Bordered = true;
		postProcessingBackground.BorderThickness = 0.01f;
		postProcessingBackground.Colour = glm::vec4(0.0f, 0.0f, 0.0f, 0.85f);
		postProcessingBackground.LeftRightUpDownExtents = glm::vec4(0.05f, 0.37f, 0.15f, 0.17f);

		UITextButton* postProcessParams = new UITextButton(std::string("Post Processing:"), glm::vec2(25.0f, (float)SCR_HEIGHT - 70.0f), glm::vec2(0.4f), glm::vec2(350.0f, 50.0f), font, glm::vec3(0.8f), 0, postProcessingBackground);
		postProcessParams->SetMouseEnterCallback(ButtonEnter);
		postProcessParams->SetMouseExitCallback(ButtonExit);
		postProcessParams->SetMouseDownCallback(ButtonPress);
		postProcessParams->SetMouseUpCallback(std::bind(&PBRScene::ParameterGroupRelease, this, std::placeholders::_1));
		postProcessParams->SetActive(false);
		group.push_back(postProcessParams);
		canvas->GetUICanvasComponent()->AddUIElement(postProcessParams);

		// Effect
		SystemRender* renderSystem = dynamic_cast<SystemRender*>(systemManager->FindSystem(SYSTEM_RENDER, RENDER_SYSTEMS));
		std::string effectString = postProcessEffectToString[renderSystem->GetPostProcess()];
		UITextButton* effect = new UITextButton(effectString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), glm::vec2(250.0f, 50.0f), font, glm::vec3(0.8f), 1);
		effect->SetMouseEnterCallback(ButtonEnter);
		effect->SetMouseExitCallback(ButtonExit);
		effect->SetMouseDownCallback(ButtonPress);
		effect->SetMouseUpCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		effect->SetActive(false);
		group.push_back(effect);
		canvas->GetUICanvasComponent()->AddUIElement(effect);

		// Post process strength
		oss << "Post Process Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetPostProcessStrength();
		std::string postStrengthString = oss.str();
		oss = std::ostringstream();
		UIText* postStrength = new UIText(postStrengthString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
		postStrength->SetActive(false);
		group.push_back(postStrength);
		canvas->GetUICanvasComponent()->AddUIElement(postStrength);

		UITextButton* postStrengthIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		postStrengthIncrease->SetMouseEnterCallback(ButtonEnter);
		postStrengthIncrease->SetMouseExitCallback(ButtonExit);
		postStrengthIncrease->SetMouseUpCallback(ButtonEnter);
		postStrengthIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
		postStrengthIncrease->SetActive(false);
		group.push_back(postStrengthIncrease);
		canvas->GetUICanvasComponent()->AddUIElement(postStrengthIncrease);

		UITextButton* postStrengthDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
		postStrengthDecrease->SetMouseEnterCallback(ButtonEnter);
		postStrengthDecrease->SetMouseExitCallback(ButtonExit);
		postStrengthDecrease->SetMouseUpCallback(ButtonEnter);
		postStrengthDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
		postStrengthDecrease->SetActive(false);
		group.push_back(postStrengthDecrease);
		canvas->GetUICanvasComponent()->AddUIElement(postStrengthDecrease);

		parameterGroups.push_back(group);
		group.clear();

		entityManager->AddEntity(canvas);
#pragma endregion

		Material* nonPBRMat = new Material();
		nonPBRMat->diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
		nonPBRMat->specular = glm::vec3(1.0f, 0.0f, 0.0f);
		nonPBRMat->shininess = 100.0f;

		Entity* nonPBRTest = new Entity("NON PBR TEST");
		nonPBRTest->AddComponent(new ComponentTransform(-5.0f, 0.35f, 2.5f));
		nonPBRTest->AddComponent(new ComponentGeometry(MODEL_CUBE));
		nonPBRTest->GetGeometryComponent()->GetModel()->ApplyMaterialToAllMesh(nonPBRMat);
		entityManager->AddEntity(nonPBRTest);

		// (5.0f, -1.0f, 2.0f)

		// Reflection probes
		std::vector<glm::vec3> positions;
		positions.push_back(glm::vec3(7.5f, 2.0f, 7.5f));
		positions.push_back(glm::vec3(7.5f, 2.0f, -7.5f));
		positions.push_back(glm::vec3(-7.5f, 2.0f, -7.5f));
		positions.push_back(glm::vec3(-7.5f, 2.0f, 7.5f));
		positions.push_back(glm::vec3(0.0f, 2.0f, 0.0f));

		positions.push_back(glm::vec3(5.0f, 0.0f, 3.5f)); // behind cart
		positions.push_back(glm::vec3(5.0f, 0.0f, 0.5f)); // infront cart
		positions.push_back(glm::vec3(7.5f, 0.0f, 2.0f)); // right cart
		positions.push_back(glm::vec3(2.5f, 0.0f, 2.0f)); // left cart
		positions.push_back(glm::vec3(5.0f, -0.8f, 2.25f)); // below cart

		// Temporary values
		std::vector<AABBPoints> localBounds;
		localBounds.push_back(AABBPoints(-17.5f, -3.0f, -17.5f, 2.5f, 3.0f, 2.5f));
		localBounds.push_back(AABBPoints(-17.5f, -3.0f, -2.5f, 2.5f, 3.0f, 17.5f));
		localBounds.push_back(AABBPoints(-2.5f, -3.0f, -2.5f, 17.5f, 3.0f, 17.5f));
		localBounds.push_back(AABBPoints(-2.5f, -3.0f, -17.5f, 17.5f, 3.0f, 2.5f));
		localBounds.push_back(AABBPoints(-10.0f, -3.0f, -10.0f, 10.0f, 3.0f, 10.0f));

		localBounds.push_back(AABBPoints(-15.0f, -1.0f, -0.25f, 5.0f, 5.0f, 6.5f));
		localBounds.push_back(AABBPoints(-15.0f, -1.0f, -9.5f, 5.0f, 5.0f, 1.25f));
		localBounds.push_back(AABBPoints(-1.5f, -1.0f, -12.0f, 2.5f, 5.0f, 9.0f));
		localBounds.push_back(AABBPoints(-12.5f, -1.0f, -12.0f, 1.5f, 5.0f, 9.0f));
		localBounds.push_back(AABBPoints(-1.0f, -0.2f, -0.75f, 1.5f, 5.8f, 0.75f));

		std::vector<float> soiRadii;
		soiRadii.push_back(7.0f);
		soiRadii.push_back(7.0f);
		soiRadii.push_back(7.0f);
		soiRadii.push_back(7.0f);
		soiRadii.push_back(15.0f);
		soiRadii.push_back(2.5f);
		soiRadii.push_back(2.5f);
		soiRadii.push_back(2.5f);
		soiRadii.push_back(2.5f);
		soiRadii.push_back(0.75f);

		std::vector<float> nearClips;
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.5f);
		nearClips.push_back(0.25f);
		nearClips.push_back(0.05f);
		nearClips.push_back(0.25f);
		nearClips.push_back(0.25f);
		nearClips.push_back(0.01f);

		std::vector<float> farClips;
		farClips.push_back(30.0f);
		farClips.push_back(30.0f);
		farClips.push_back(30.0f);
		farClips.push_back(30.0f);
		farClips.push_back(30.0f);
		farClips.push_back(4.5f);
		farClips.push_back(4.5f);
		farClips.push_back(4.5f);
		farClips.push_back(4.5f);
		farClips.push_back(2.5f);

		std::vector<bool> renderSkybox;
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(true);
		renderSkybox.push_back(false);
		renderSkybox.push_back(false);
		renderSkybox.push_back(false);
		renderSkybox.push_back(false);
		renderSkybox.push_back(false);

		RenderManager::GetInstance()->GetBakedData().InitialiseReflectionProbes(positions, localBounds, soiRadii, nearClips, farClips, renderSkybox, name, 1024u);
	}

	void PBRScene::CreateSystems()
	{
		systemManager->AddSystem(new SystemPhysics(), UPDATE_SYSTEMS);
		SystemRender* renderSystem = new SystemRender();
		renderSystem->SetPostProcess(PostProcessingEffect::NONE);
		renderSystem->SetActiveCamera(camera);
		systemManager->AddSystem(renderSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemShadowMapping(), RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemUIRender(), RENDER_SYSTEMS);
		systemManager->AddCollisionResponseSystem(new CollisionResolver(collisionManager));
		systemManager->AddSystem(new SystemUIMouseInteraction(inputManager), UPDATE_SYSTEMS);
		systemManager->AddSystem(new SystemFrustumCulling(camera, collisionManager), UPDATE_SYSTEMS);
		SystemReflectionBaking* reflectionSystem = new SystemReflectionBaking();
		reflectionSystem->SetActiveCamera(camera);
		systemManager->AddSystem(reflectionSystem, RENDER_SYSTEMS);
		systemManager->AddSystem(new SystemRenderColliders(collisionManager), RENDER_SYSTEMS);
	}

	void PBRScene::Update()
	{
		Scene::Update();
		systemManager->ActionUpdateSystems(entityManager);
	
		float time = (float)glfwGetTime();

		//dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Spot Parent")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 25.0f);

		glm::vec3 axis = dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Spot Light")->GetComponent(COMPONENT_TRANSFORM))->RotationAxis();
		//axis.x += 0.3 * Scene::dt * sin(time);
		//axis.y *= 0.3 * Scene::dt * sin(time);
		//axis.z += 0.3 * Scene::dt * cos(time);
		axis = glm::normalize(axis);
		dynamic_cast<ComponentTransform*>(entityManager->FindEntity("Spot Light")->GetComponent(COMPONENT_TRANSFORM))->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 50.0f);

		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) {
			targetFPSPercentage = 1.0f;
		}

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(entityManager->FindEntity("Canvas")->GetUICanvasComponent()->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));
	}

	void PBRScene::Render()
	{
		Scene::Render();
		systemManager->ActionRenderSystems(entityManager, SCR_WIDTH, SCR_HEIGHT);
	}

	void PBRScene::Close()
	{
		std::cout << "Closing PBR scene" << std::endl;
	}

	void PBRScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		if (key == GLFW_KEY_C) {
			for (UIElement* ui : optionButtons) {
				ui->SetActive(!ui->GetActive());
			}
			for (UIElement* ui : parameterGroups[parameterGroupIndex]) {
				ui->SetActive(!ui->GetActive());
			}
		}
	}

	void PBRScene::keyDown(int key)
	{

	}

	void PBRScene::ChangePostProcessEffect()
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
}