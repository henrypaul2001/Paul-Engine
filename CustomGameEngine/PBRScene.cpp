#include "PBRScene.h"
#include "GameInputManager.h"
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

		resources->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
		renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");

		//systemManager->BakeReflectionProbes(entityManager->Entities());
		renderManager->GetBakedData().LoadReflectionProbesFromFile();
		rebuildBVHOnUpdate = true;
	}

	PBRScene::~PBRScene() {}

	void PBRScene::SetupScene()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();

		resources->LoadHDREnvironmentMap("Textures/Environment Maps/st_peters_square_night.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/newport_loft.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/sky.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/metro_noord.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/laufenurg_church.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/golden_bay.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/metro_vijzelgracht.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/starsky.hdr", true);
		//resources->LoadHDREnvironmentMap("Textures/Environment Maps/studio.hdr", true);

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

		ComponentLight* directionalLight = ecs.GetComponent<ComponentLight>(directionalLightEntity);

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
			SystemRender& renderSystem = renderManager->GetRenderPipeline()->GetRenderSystem();
			unsigned int currentEffect;
			switch (buttonId) {
			case 1:
				ChangePostProcessEffect();

				currentEffect = renderSystem.GetPostProcess();
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

		ComponentLight* directionalLight = ecs.GetComponent<ComponentLight>(directionalLightEntity);

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
		SCOPE_TIMER("PBRScene::CreateEntities");
		ambientStrength = 0.08f;

		Entity* dirLight = ecs.New("Directional Light");
		directionalLightEntity = dirLight->ID();
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		//directional.Ambient = glm::vec3(0.01f, 0.01f, 0.05f);
		//directional.Ambient = glm::vec3(0.035f, 0.035f, 0.08f);
		directional.Colour = glm::vec3(5.9f, 5.1f, 9.5f);
		directional.Ambient = directional.Colour * ambientStrength;
		//directional.Colour = glm::vec3(0.0f);
		directional.Direction = glm::vec3(-1.0f, -0.9f, 1.0f);
		directional.MinShadowBias = 0.0f;
		directional.MaxShadowBias = 0.003f;
		directional.DirectionalLightDistance = 20.0f;
		ecs.AddComponent(directionalLightEntity, directional);

		PBRMaterial* bloomTest;
		PBRMaterial* mirror;
		PBRMaterial* gold;
		PBRMaterial* rusted_iron;
		PBRMaterial* bricks;
		PBRMaterial* scifi;
		PBRMaterial* snow;
		PBRMaterial* space_blanket;
		PBRMaterial* wall;
		PBRMaterial* raindrops;
		{
			SCOPE_TIMER("PBRScene::CreateEntities::Materials");
#pragma region materials
			bloomTest = new PBRMaterial(glm::vec3(1500.0f));
			bloomTest->metallic = 0.0f;
			bloomTest->roughness = 0.0f;
			bloomTest->ao = 0.0f;
			resources->AddMaterial("Bloom Test", bloomTest);

			mirror = new PBRMaterial(glm::vec3(0.0f));
			mirror->metallic = 0.1f;
			mirror->roughness = 0.0f;
			mirror->ao = 1.0f;
			resources->AddMaterial("Mirror", mirror);

			gold = new PBRMaterial();
			gold->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/gold/albedo.png", TEXTURE_ALBEDO, true));
			gold->normalMaps.push_back(resources->LoadTexture("Materials/PBR/gold/normal.png", TEXTURE_NORMAL, false));
			gold->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/gold/metallic.png", TEXTURE_METALLIC, false));
			gold->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/gold/roughness.png", TEXTURE_ROUGHNESS, false));
			gold->aoMaps.push_back(resources->LoadTexture("Materials/PBR/gold/ao.png", TEXTURE_AO, false));
			resources->AddMaterial("Gold", gold);

			rusted_iron = new PBRMaterial();
			rusted_iron->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/rusted_iron/albedo.png", TEXTURE_ALBEDO, true));
			rusted_iron->normalMaps.push_back(resources->LoadTexture("Materials/PBR/rusted_iron/normal.png", TEXTURE_NORMAL, false));
			rusted_iron->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/rusted_iron/metallic.png", TEXTURE_METALLIC, false));
			rusted_iron->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/rusted_iron/roughness.png", TEXTURE_ROUGHNESS, false));
			rusted_iron->aoMaps.push_back(resources->LoadTexture("Materials/PBR/rusted_iron/ao.png", TEXTURE_AO, false));
			resources->AddMaterial("Rusted Iron", rusted_iron);

			//PBRMaterial* plastic = new PBRMaterial();
			//plastic->albedoMaps.push_back(resources->LoadTexture("Materials/PBR/plastic/albedo.png", TEXTURE_ALBEDO, true));
			//plastic->normalMaps.push_back(resources->LoadTexture("Materials/PBR/plastic/normal.png", TEXTURE_NORMAL, false));
			//plastic->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/plastic/metallic.png", TEXTURE_METALLIC, false));
			//plastic->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/plastic/roughness.png", TEXTURE_ROUGHNESS, false));
			//plastic->aoMaps.push_back(resources->LoadTexture("Materials/PBR/plastic/ao.png", TEXTURE_AO, false));

			bricks = new PBRMaterial();
			bricks->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/albedo.png", TEXTURE_ALBEDO, true));
			bricks->normalMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/normal.png", TEXTURE_NORMAL, false));
			bricks->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/specular.png", TEXTURE_METALLIC, false));
			bricks->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/roughness.png", TEXTURE_ROUGHNESS, false));
			bricks->aoMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/ao.png", TEXTURE_AO, false));
			bricks->heightMaps.push_back(resources->LoadTexture("Materials/PBR/bricks/displacement.png", TEXTURE_DISPLACE, false));
			bricks->height_scale = -0.1;
			bricks->textureScaling = glm::vec2(10.0f);
			resources->AddMaterial("Bricks", bricks);

			//PBRMaterial* grass = new PBRMaterial();
			//grass->albedoMaps.push_back(resources->LoadTexture("Materials/PBR/grass/albedo.png", TEXTURE_ALBEDO, true));
			//grass->normalMaps.push_back(resources->LoadTexture("Materials/PBR/grass/normal.png", TEXTURE_NORMAL, false));
			//grass->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/grass/metallic.png", TEXTURE_METALLIC, false));
			//grass->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/grass/roughness.png", TEXTURE_ROUGHNESS, false));
			//grass->aoMaps.push_back(resources->LoadTexture("Materials/PBR/grass/ao.png", TEXTURE_AO, false));

			//PBRMaterial* leather = new PBRMaterial();
			//leather->albedoMaps.push_back(resources->LoadTexture("Materials/PBR/leather/albedo.png", TEXTURE_ALBEDO, true));
			//leather->normalMaps.push_back(resources->LoadTexture("Materials/PBR/leather/normal.png", TEXTURE_NORMAL, false));
			//leather->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/leather/roughness.png", TEXTURE_ROUGHNESS, false));

			//PBRMaterial* metal_plate = new PBRMaterial();
			//metal_plate->albedoMaps.push_back(resources->LoadTexture("Materials/PBR/metal_plate/albedo.png", TEXTURE_ALBEDO, true));
			//metal_plate->normalMaps.push_back(resources->LoadTexture("Materials/PBR/metal_plate/normal.png", TEXTURE_NORMAL, false));
			//metal_plate->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/metal_plate/metallic.png", TEXTURE_METALLIC, false));
			//metal_plate->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/metal_plate/roughness.png", TEXTURE_ROUGHNESS, false));
			//metal_plate->aoMaps.push_back(resources->LoadTexture("Materials/PBR/metal_plate/ao.png", TEXTURE_AO, false));
			//metal_plate->heightMaps.push_back(resources->LoadTexture("Materials/PBR/metal_plate/displacement.png", TEXTURE_DISPLACE, false));
			//metal_plate->height_scale = -0.1;

			scifi = new PBRMaterial();
			scifi->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/scifi/albedo.png", TEXTURE_ALBEDO, true));
			scifi->normalMaps.push_back(resources->LoadTexture("Materials/PBR/scifi/normal.png", TEXTURE_NORMAL, false));
			scifi->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/scifi/metallic.png", TEXTURE_METALLIC, false));
			scifi->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/scifi/roughness.png", TEXTURE_ROUGHNESS, false));
			scifi->aoMaps.push_back(resources->LoadTexture("Materials/PBR/scifi/ao.png", TEXTURE_AO, false));
			//scifi->heightMaps.push_back(resources->LoadTexture("Materials/PBR/scifi/displacement.png", TEXTURE_DISPLACE));
			//scifi->height_scale = -0.1;
			resources->AddMaterial("SciFi", scifi);

			snow = new PBRMaterial();
			snow->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/snow/albedo.png", TEXTURE_ALBEDO, true));
			snow->normalMaps.push_back(resources->LoadTexture("Materials/PBR/snow/normal.png", TEXTURE_NORMAL, false));
			snow->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/snow/specular.png", TEXTURE_METALLIC, false));
			snow->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/snow/roughness.png", TEXTURE_ROUGHNESS, false));
			snow->aoMaps.push_back(resources->LoadTexture("Materials/PBR/snow/ao.png", TEXTURE_AO, false));
			snow->heightMaps.push_back(resources->LoadTexture("Materials/PBR/snow/displacement.png", TEXTURE_DISPLACE, false));
			snow->height_scale = -0.1;
			resources->AddMaterial("Snow", snow);

			space_blanket = new PBRMaterial();
			space_blanket->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/space_blanket/albedo.png", TEXTURE_ALBEDO, true));
			space_blanket->normalMaps.push_back(resources->LoadTexture("Materials/PBR/space_blanket/normal.png", TEXTURE_NORMAL, false));
			space_blanket->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/space_blanket/metallic.png", TEXTURE_METALLIC, false));
			space_blanket->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/space_blanket/roughness.png", TEXTURE_ROUGHNESS, false));
			space_blanket->aoMaps.push_back(resources->LoadTexture("Materials/PBR/space_blanket/ao.png", TEXTURE_AO, false));
			resources->AddMaterial("Space Blanket", space_blanket);

			wall = new PBRMaterial();
			wall->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/wall/albedo.png", TEXTURE_ALBEDO, false));
			wall->normalMaps.push_back(resources->LoadTexture("Materials/PBR/wall/normal.png", TEXTURE_NORMAL, false));
			wall->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/wall/metallic.png", TEXTURE_METALLIC, false));
			wall->roughnessMaps.push_back(resources ->LoadTexture("Materials/PBR/wall/roughness.png", TEXTURE_ROUGHNESS, false));
			wall->aoMaps.push_back(resources->LoadTexture("Materials/PBR/wall/ao.png", TEXTURE_AO, false));
			resources->AddMaterial("Wall", wall);

			//PBRMaterial* worn_corrugated_iron = new PBRMaterial();
			//worn_corrugated_iron->albedoMaps.push_back(resources->LoadTexture("Materials/PBR/worn_corrugated_iron/albedo.png", TEXTURE_ALBEDO, true));
			//worn_corrugated_iron->normalMaps.push_back(resources->LoadTexture("Materials/PBR/worn_corrugated_iron/normal.png", TEXTURE_NORMAL, false));
			//worn_corrugated_iron->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/worn_corrugated_iron/roughness.png", TEXTURE_ROUGHNESS, false));
			//worn_corrugated_iron->aoMaps.push_back(resources->LoadTexture("Materials/PBR/worn_corrugated_iron/ao.png", TEXTURE_AO, false));
			//worn_corrugated_iron->heightMaps.push_back(resources->LoadTexture("Materials/PBR/worn_corrugated_iron/displacement.png", TEXTURE_DISPLACE, false));
			//worn_corrugated_iron->height_scale = -0.1;

			//PBRMaterial* earth = new PBRMaterial();
			//earth->albedoMaps.push_back(resources->LoadTexture("Materials/PBR/earth/albedo.jpg", TEXTURE_ALBEDO, true));
			//earth->heightMaps.push_back(resources->LoadTexture("Materials/PBR/earth/displacement.jpg", TEXTURE_DISPLACE, false));
			//earth->metallicMaps.push_back(resources->LoadTexture("Materials/PBR/earth/specular.jpg", TEXTURE_METALLIC, false));
			//earth->height_scale = -0.1;

			raindrops = new PBRMaterial();
			raindrops->baseColourMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/albedo.jpg", TEXTURE_ALBEDO, true));
			raindrops->normalMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/normal.png", TEXTURE_NORMAL, false));
			raindrops->roughnessMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/roughness.jpg", TEXTURE_ROUGHNESS, false));
			raindrops->aoMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/ao.jpg", TEXTURE_AO, false));
			raindrops->heightMaps.push_back(resources->LoadTexture("Materials/PBR/rain_drops/height.png", TEXTURE_DISPLACE, false));
			raindrops->PushOpacityMap(resources->LoadTexture("Materials/PBR/rain_drops/opacity.png", TEXTURE_OPACITY, false));
			raindrops->shadowCastAlphaDiscardThreshold = 1.0f;
			raindrops->textureScaling = glm::vec2(10.0f);
			resources->AddMaterial("Raindrops", raindrops);
#pragma endregion
		}

		{
			SCOPE_TIMER("PBRScene::CreateEntities::Scene");
#pragma region scene
			Entity* floor = ecs.New("Floor");
			ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(floor->ID());
			transform->SetPosition(glm::vec3(0.0f, -1.0f, 0.0));
			ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_PLANE, true));
			ComponentGeometry* geometry = ecs.GetComponent<ComponentGeometry>(floor->ID());
			geometry->ApplyMaterialToModel(bricks);
			geometry->SetTextureScale(10.0f);
			transform->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
			transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);

			Entity* ceiling = ecs.New("Cieling");
			transform = ecs.GetComponent<ComponentTransform>(ceiling->ID());
			transform->SetPosition(glm::vec3(0.0f, 5.0f, 0.0));
			ecs.AddComponent(ceiling->ID(), ComponentGeometry(MODEL_PLANE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(ceiling->ID());
			geometry->ApplyMaterialToModel(bricks);
			geometry->SetTextureScale(10.0f);
			transform->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
			transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), 90.0f);

			Entity* rainFloor = ecs.New("Rain Floor");
			transform = ecs.GetComponent<ComponentTransform>(rainFloor->ID());
			transform->SetPosition(glm::vec3(0.0f, -0.99f, 0.0f));
			ecs.AddComponent(rainFloor->ID(), ComponentGeometry(MODEL_PLANE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(rainFloor->ID());
			geometry->ApplyMaterialToModel(raindrops);
			geometry->SetTextureScale(10.0f);
			transform->SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
			transform->SetRotation(glm::vec3(1.0, 0.0, 0.0), -90.0f);

			Entity* wall1 = ecs.New("Wall 1");
			transform = ecs.GetComponent<ComponentTransform>(wall1->ID());
			transform->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));
			ecs.AddComponent(wall1->ID(), ComponentGeometry(MODEL_PLANE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(wall1->ID());
			geometry->ApplyMaterialToModel(snow);
			geometry->SetTextureScale(2.0f);
			geometry->SetCulling(false, GL_BACK);
			transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
			transform->SetRotation(glm::vec3(0.0, 1.0, 0.0), 180.0f);

			Entity* wall2 = ecs.New("Wall 2");
			transform = ecs.GetComponent<ComponentTransform>(wall2->ID());
			transform->SetPosition(glm::vec3(0.0f, 0.0f, -10.0f));
			ecs.AddComponent(wall2->ID(), ComponentGeometry(MODEL_PLANE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(wall2->ID());
			geometry->ApplyMaterialToModel(space_blanket);
			geometry->SetTextureScale(2.0f);
			geometry->SetCulling(false, GL_BACK);
			transform ->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));

			Entity* wall3 = ecs.New("Wall 3");
			transform = ecs.GetComponent<ComponentTransform>(wall3->ID());
			transform->SetPosition(glm::vec3(10.0f, 0.0f, 0.0f));
			ecs.AddComponent(wall3->ID(), ComponentGeometry(MODEL_PLANE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(wall3->ID());
			geometry->ApplyMaterialToModel(rusted_iron);
			geometry->SetTextureScale(2.0f);
			geometry->SetCulling(false, GL_BACK);
			transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
			transform->SetRotation(glm::vec3(0.0, 1.0, 0.0), -90.0f);

			Entity* wall4 = ecs.New("Wall 4");
			transform = ecs.GetComponent<ComponentTransform>(wall4->ID());
			transform->SetPosition(glm::vec3(-10.0f, 0.0f, 0.0f));
			ecs.AddComponent(wall4->ID(), ComponentGeometry(MODEL_PLANE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(wall4->ID());
			geometry->ApplyMaterialToModel(scifi);
			geometry->SetTextureScale(2.0f);
			geometry->SetCulling(false, GL_BACK);
			transform->SetScale(glm::vec3(13.0f, 5.0f, 1.0f));
			transform->SetRotation(glm::vec3(0.0, 1.0, 0.0), 90.0f);

			Entity* pointLight = ecs.New("Point Light");
			transform = ecs.GetComponent<ComponentTransform>(pointLight->ID());
			transform->SetPosition(glm::vec3(6.5f, 4.0f, -6.5f));
			ecs.AddComponent(pointLight->ID(), ComponentGeometry(MODEL_SPHERE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(pointLight->ID());
			geometry->ApplyMaterialToModel(rusted_iron);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(0.25f));
			ComponentLight light = ComponentLight(POINT);
			light.Colour = glm::vec3(50.0, 50.0, 50.0);
			//light.Colour = glm::vec3(1.0, 1.0, 1.0);
			light.CastShadows = true;
			light.Active = true;
			ecs.AddComponent(pointLight->ID(), light);

			Entity* pointLight2 = ecs.New("Point Light2");
			transform = ecs.GetComponent<ComponentTransform>(pointLight2->ID());
			transform->SetPosition(glm::vec3(-8.5f, 4.0f, 8.5f));
			ecs.AddComponent(pointLight2->ID(), ComponentGeometry(MODEL_SPHERE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(pointLight2->ID());
			geometry->ApplyMaterialToModel(rusted_iron);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(0.25f));
			ComponentLight light2 = ComponentLight(POINT);
			light2.Colour = glm::vec3(150.0, 150.0, 150.0);
			//light2.Colour = glm::vec3(1.0, 1.0, 1.0);
			light2.CastShadows = true;
			light2.Active = true;
			ecs.AddComponent(pointLight2->ID(), light2);

			Entity* pointLight3 = ecs.New("Point Light3");
			transform = ecs.GetComponent<ComponentTransform>(pointLight3->ID());
			transform->SetPosition(glm::vec3(6.5f, 4.0f, 6.5f));
			ecs.AddComponent(pointLight3->ID(), ComponentGeometry(MODEL_SPHERE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(pointLight3->ID());
			geometry->ApplyMaterialToModel(rusted_iron);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(0.25f));
			ComponentLight light3 = ComponentLight(POINT);
			light3.Colour = glm::vec3(50.0, 50.0, 50.0);
			//light3.Colour = glm::vec3(1.0, 1.0, 1.0);
			light3.CastShadows = true;
			light3.Active = true;
			ecs.AddComponent(pointLight3->ID(), light3);

			Entity* pointLight4 = ecs.New("Point Light4");
			transform = ecs.GetComponent<ComponentTransform>(pointLight4->ID());
			transform->SetPosition(glm::vec3(-6.5f, 2.0f, -6.5f));
			ecs.AddComponent(pointLight4->ID(), ComponentGeometry(MODEL_SPHERE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(pointLight4->ID());
			geometry->ApplyMaterialToModel(rusted_iron);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(0.25f));
			ComponentLight light4 = ComponentLight(POINT);
			light4.Colour = glm::vec3(50.0, 50.0, 50.0);
			//light4.Colour = glm::vec3(1.0, 1.0, 1.0);
			light4.CastShadows = true;
			light4.Active = true;
			ecs.AddComponent(pointLight4->ID(), light4);

			Entity* pointLight5 = ecs.New("Point Light5");
			transform = ecs.GetComponent<ComponentTransform>(pointLight5->ID());
			transform->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
			ecs.AddComponent(pointLight5->ID(), ComponentGeometry(MODEL_SPHERE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(pointLight5->ID());
			geometry->ApplyMaterialToModel(rusted_iron);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(0.25f));
			ComponentLight light5 = ComponentLight(POINT);
			light5.Colour = glm::vec3(25.0f, 25.0f, 25.0f);
			//light5.Colour = glm::vec3(0.5, 0.5, 0.5);
			light5.Linear = 0.027f;
			light5.Quadratic = 0.0028f;
			light5.CastShadows = true;
			light5.Active = true;
			ecs.AddComponent(pointLight5->ID(), light5);

			Entity* spotParent = ecs.New("Spot Parent");
			transform = ecs.GetComponent<ComponentTransform>(spotParent->ID());
			transform->SetPosition(glm::vec3(1.0f, 0.0f, 4.6f));
			ecs.AddComponent(spotParent->ID(), ComponentGeometry(MODEL_CUBE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(spotParent->ID());
			geometry->ApplyMaterialToModel(raindrops);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(1.0f));
			const unsigned int parentID = spotParent->ID();

			Entity* spotLight = ecs.New("Spot Light");
			//ecs.GetComponent<ComponentTransform>(parentID)->AddChild(spotLight->ID());
			transform = ecs.GetComponent<ComponentTransform>(spotLight->ID());
			transform->SetPosition(glm::vec3(-5.5f, 1.0f, 0.0f));
			ecs.AddComponent(spotLight->ID(), ComponentGeometry(MODEL_CUBE, true));
			geometry = ecs.GetComponent<ComponentGeometry>(spotLight->ID());
			geometry->ApplyMaterialToModel(gold);
			geometry->CastShadows(true);
			transform->SetScale(glm::vec3(0.25f));
			ComponentLight spot = ComponentLight(SPOT);
			spot.Colour = glm::vec3(70.0f, 20.0f, 40.0f);
			//spot.Colour = glm::vec3(0.7f, 0.2f, 0.4f);
			spot.CastShadows = true;
			spot.Direction = glm::vec3(-1.0f, 0.0f, 0.0f);
			spot.Cutoff = glm::cos(glm::radians(20.0f));
			spot.OuterCutoff = glm::cos(glm::radians(32.0f));
			ecs.AddComponent(spotLight->ID(), spot);

			Entity* goblet = ecs.New("Goblet");
			transform = ecs.GetComponent<ComponentTransform>(goblet->ID());
			transform->SetPosition(glm::vec3(0.0f, 0.35f, 0.0f));
			transform->SetScale(glm::vec3(4.0f));
			ecs.AddComponent(goblet->ID(), ComponentGeometry("Models/PBR/brass_goblet/brass_goblet.obj", true));
			const unsigned int gobletID = goblet->ID();

			Entity* gobletClone = ecs.Clone(gobletID);
			transform = ecs.GetComponent<ComponentTransform>(gobletClone->ID());
			transform->SetPosition(transform->GetWorldPosition() + glm::vec3(0.0f, 5.0f, 0.0f));
			geometry = ecs.GetComponent<ComponentGeometry>(gobletClone->ID());
			geometry->ApplyMaterialToModel(mirror);

			Entity* cart = ecs.New("Cart");
			transform = ecs.GetComponent<ComponentTransform>(cart->ID());
			transform->SetPosition(glm::vec3(5.0f, -1.0f, 2.0f));
			transform->SetScale(glm::vec3(2.0f));
			ecs.AddComponent(cart->ID(), ComponentGeometry("Models/PBR/cart/cart.obj", true));

			//Entity* bloomCube = ecs.New("Bloom Cube");
			//transform = ecs.GetComponent<ComponentTransform>(bloomCube->ID());
			//transform->SetPosition(glm::vec3(-2.5f, 0.35f, 2.5f));
			//transform->SetScale(glm::vec3(0.5f));
			//ecs.AddComponent(bloomCube->ID(), ComponentGeometry(MODEL_CUBE, true));
			//ComponentLight bloomLight = ComponentLight(POINT);
			////bloomLight.Colour = glm::vec3(50.0f, 50.0f, 50.0f);
			//bloomLight.Colour = glm::vec3(0.5f, 0.5f, 0.5f);
			//bloomLight.CastShadows = false;
			//ecs.AddComponent(bloomCube->ID(), bloomLight);
			//ecs.GetComponent<ComponentGeometry>(bloomCube->ID())->GetModel()->ApplyMaterialsToAllMesh({ bloomTest });
#pragma endregion
		}

		{
			SCOPE_TIMER("PBRScene::CreateEntities::UI");
#pragma region ui
			TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
			Entity* uiCanvas = ecs.New("Canvas");
			ecs.AddComponent(uiCanvas->ID(), ComponentUICanvas(SCREEN_SPACE));
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());
			canvas->AddUIElement(new UIText(std::string("Paul Engine"), glm::vec2(25.0f, 135.0f), glm::vec2(0.25f, 0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
			canvas->AddUIElement(new UIText(std::string("FPS: "), glm::vec2(25.0f, 10.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f)));
			canvas->AddUIElement(new UIText(std::string("Resolution: ") + std::to_string(SCR_WIDTH) + " X " + std::to_string(SCR_HEIGHT), glm::vec2(25.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));
			canvas->AddUIElement(new UIText(std::string("Shadow res: ") + std::to_string(renderManager->ShadowWidth()) + " X " + std::to_string(renderManager->ShadowHeight()), glm::vec2(25.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

			const std::string renderPipeline = renderManager->GetRenderPipeline()->PipelineName();
			canvas->AddUIElement(new UIText(renderPipeline, glm::vec2(25.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f)));

			// Geometry debug UI
			UIBackground geometryDebugBackground;
			geometryDebugBackground.Colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			geometryDebugBackground.LeftRightUpDownExtents = glm::vec4(0.01f, 0.225f, 0.05f, 0.8f);
			geometryDebugBackground.BorderColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			geometryDebugBackground.BorderThickness = 0.01f;
			geometryDebugBackground.Bordered = true;

			UIText* geoDebugText = new UIText(std::string("Geometry Debug"), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 135.0f), glm::vec2(0.25f), font, glm::vec3(0.0f, 0.0f, 0.0f));
			geoDebugText->SetBackground(geometryDebugBackground);
			geoDebugText->UseBackground(true);
			geoDebugText->SetActive(false);
			UIText* meshCountText = new UIText(std::string("Mesh count: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 105.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
			meshCountText->SetActive(false);
			UIText* visibleCountText = new UIText(std::string("     - Visible: "), glm::vec2((SCR_WIDTH / 2.0f) - 145.0f, 75.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
			visibleCountText->SetActive(false);
			UIText* bvhCountText = new UIText(std::string("BVHN count: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 45.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
			bvhCountText->SetActive(false);
			UIText* aabbTestCountText = new UIText(std::string("AABB Tests: "), glm::vec2((SCR_WIDTH / 2.0f) - 150.0f, 15.0f), glm::vec2(0.17f), font, glm::vec3(0.0f, 0.0f, 0.0f));
			aabbTestCountText->SetActive(false);
			canvas->AddUIElement(geoDebugText);
			canvas->AddUIElement(meshCountText);
			canvas->AddUIElement(visibleCountText);
			canvas->AddUIElement(bvhCountText);
			canvas->AddUIElement(aabbTestCountText);

			// Bloom button
			UITextButton* bloomBtn = new UITextButton(std::string("Bloom: adv + dirt"), glm::vec2(355.0f, 60.0f), glm::vec2(0.4f, 0.4f), glm::vec2(445.0f, 50.0f), font, glm::vec3(0.8f, 0.8f, 0.8f), 0);
			bloomBtn->SetMouseEnterCallback(ButtonEnter);
			bloomBtn->SetMouseExitCallback(ButtonExit);
			bloomBtn->SetMouseDownCallback(ButtonPress);
			bloomBtn->SetMouseUpCallback(BloomBtnRelease);
			bloomBtn->SetActive(false);
			optionButtons.push_back(bloomBtn);
			canvas->AddUIElement(bloomBtn);

			// SSAO button
			UITextButton* ssaoBtn = new UITextButton(std::string("SSAO: on"), glm::vec2(850.0f, 60.0f), glm::vec2(0.4f), glm::vec2(260.0f, 50.0f), font, glm::vec3(0.8f), 0);
			ssaoBtn->SetMouseEnterCallback(ButtonEnter);
			ssaoBtn->SetMouseExitCallback(ButtonExit);
			ssaoBtn->SetMouseDownCallback(ButtonPress);
			ssaoBtn->SetMouseUpCallback(SSAOBtnRelease);
			ssaoBtn->SetActive(false);
			optionButtons.push_back(ssaoBtn);
			canvas->AddUIElement(ssaoBtn);

			// Shadows button
			UITextButton* shadowsBtn = new UITextButton(std::string("Shadows: on"), glm::vec2(1170.0f, 60.0f), glm::vec2(0.4f), glm::vec2(330.0f, 50.0f), font, glm::vec3(0.8f), 0);
			shadowsBtn->SetMouseEnterCallback(ButtonEnter);
			shadowsBtn->SetMouseExitCallback(ButtonExit);
			shadowsBtn->SetMouseDownCallback(ButtonPress);
			shadowsBtn->SetMouseUpCallback(ShadowsBtnRelease);
			shadowsBtn->SetActive(false);
			optionButtons.push_back(shadowsBtn);
			canvas->AddUIElement(shadowsBtn);

			// IBL button
			UITextButton* iblBtn = new UITextButton(std::string("IBL: on"), glm::vec2(1550.0f, 60.0f), glm::vec2(0.4f), glm::vec2(185.0f, 50.0f), font, glm::vec3(0.8f), 0);
			iblBtn->SetMouseEnterCallback(ButtonEnter);
			iblBtn->SetMouseExitCallback(ButtonExit);
			iblBtn->SetMouseDownCallback(ButtonPress);
			iblBtn->SetMouseUpCallback(IBLBtnRelease);
			iblBtn->SetActive(false);
			optionButtons.push_back(iblBtn);
			canvas->AddUIElement(iblBtn);

			// Env map button
			UITextButton* envMapBtn = new UITextButton(std::string("Switch Env Map"), glm::vec2(1900.0f, 60.0f), glm::vec2(0.4f), glm::vec2(420.0f, 50.0f), font, glm::vec3(0.8f), 0);
			envMapBtn->SetMouseEnterCallback(ButtonEnter);
			envMapBtn->SetMouseExitCallback(ButtonExit);
			envMapBtn->SetMouseDownCallback(ButtonPress);
			envMapBtn->SetMouseUpCallback(std::bind(&PBRScene::EnvMapBtnRelease, this, std::placeholders::_1));
			envMapBtn->SetActive(false);
			optionButtons.push_back(envMapBtn);
			canvas->AddUIElement(envMapBtn);

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
			canvas->AddUIElement(tonemappingParams);

			// Exposure
			oss << "Exposure: " << std::setprecision(4) << renderManager->GetRenderParams()->GetExposure();
			std::string exposureString = oss.str();
			oss = std::ostringstream();
			UIText* exposure = new UIText(exposureString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			exposure->SetActive(false);
			group.push_back(exposure);
			canvas->AddUIElement(exposure);

			UITextButton* exposureIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			exposureIncrease->SetMouseEnterCallback(ButtonEnter);
			exposureIncrease->SetMouseExitCallback(ButtonExit);
			exposureIncrease->SetMouseUpCallback(ButtonEnter);
			exposureIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			exposureIncrease->SetActive(false);
			group.push_back(exposureIncrease);
			canvas->AddUIElement(exposureIncrease);

			UITextButton* exposureDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			exposureDecrease->SetMouseEnterCallback(ButtonEnter);
			exposureDecrease->SetMouseExitCallback(ButtonExit);
			exposureDecrease->SetMouseUpCallback(ButtonEnter);
			exposureDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			exposureDecrease->SetActive(false);
			group.push_back(exposureDecrease);
			canvas->AddUIElement(exposureDecrease);

			// Gamma
			UIText* gamma = new UIText(std::string("Gamma: " + std::to_string(renderManager->GetRenderParams()->GetGamma())), glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			gamma->SetActive(false);
			group.push_back(gamma);
			canvas->AddUIElement(gamma);

			UITextButton* gammaIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			gammaIncrease->SetMouseEnterCallback(ButtonEnter);
			gammaIncrease->SetMouseExitCallback(ButtonExit);
			gammaIncrease->SetMouseUpCallback(ButtonEnter);
			gammaIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			gammaIncrease->SetActive(false);
			group.push_back(gammaIncrease);
			canvas->AddUIElement(gammaIncrease);

			UITextButton* gammaDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			gammaDecrease->SetMouseEnterCallback(ButtonEnter);
			gammaDecrease->SetMouseExitCallback(ButtonExit);
			gammaDecrease->SetMouseUpCallback(ButtonEnter);
			gammaDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			gammaDecrease->SetActive(false);
			group.push_back(gammaDecrease);
			canvas->AddUIElement(gammaDecrease);

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
			canvas->AddUIElement(advBloomParams);

			// Threshold
			oss << "Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomThreshold();
			std::string thresholdString = oss.str();
			oss = std::ostringstream();
			UIText* threshold = new UIText(thresholdString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			threshold->SetActive(false);
			group.push_back(threshold);
			canvas->AddUIElement(threshold);

			UITextButton* thresholdIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			thresholdIncrease->SetMouseEnterCallback(ButtonEnter);
			thresholdIncrease->SetMouseExitCallback(ButtonExit);
			thresholdIncrease->SetMouseUpCallback(ButtonEnter);
			thresholdIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			thresholdIncrease->SetActive(false);
			group.push_back(thresholdIncrease);
			canvas->AddUIElement(thresholdIncrease);

			UITextButton* thresholdDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			thresholdDecrease->SetMouseEnterCallback(ButtonEnter);
			thresholdDecrease->SetMouseExitCallback(ButtonExit);
			thresholdDecrease->SetMouseUpCallback(ButtonEnter);
			thresholdDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			thresholdDecrease->SetActive(false);
			group.push_back(thresholdDecrease);
			canvas->AddUIElement(thresholdDecrease);

			// Soft threshold
			oss << "Soft Threshold: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomSoftThreshold();
			std::string softThresholdString = oss.str();
			oss = std::ostringstream();
			UIText* softThreshold = new UIText(softThresholdString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			softThreshold->SetActive(false);
			group.push_back(softThreshold);
			canvas->AddUIElement(softThreshold);

			UITextButton* softThresholdIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			softThresholdIncrease->SetMouseEnterCallback(ButtonEnter);
			softThresholdIncrease->SetMouseExitCallback(ButtonExit);
			softThresholdIncrease->SetMouseUpCallback(ButtonEnter);
			softThresholdIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			softThresholdIncrease->SetActive(false);
			group.push_back(softThresholdIncrease);
			canvas->AddUIElement(softThresholdIncrease);

			UITextButton* softThresholdDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			softThresholdDecrease->SetMouseEnterCallback(ButtonEnter);
			softThresholdDecrease->SetMouseExitCallback(ButtonExit);
			softThresholdDecrease->SetMouseUpCallback(ButtonEnter);
			softThresholdDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			softThresholdDecrease->SetActive(false);
			group.push_back(softThresholdDecrease);
			canvas->AddUIElement(softThresholdDecrease);

			// Filter radius
			oss << "Filter Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomFilterRadius();
			std::string filterRadiusString = oss.str();
			oss = std::ostringstream();
			UIText* filterRadius = new UIText(filterRadiusString, glm::vec2(60.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			filterRadius->SetActive(false);
			group.push_back(filterRadius);
			canvas->AddUIElement(filterRadius);

			UITextButton* filterRadiusIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
			filterRadiusIncrease->SetMouseEnterCallback(ButtonEnter);
			filterRadiusIncrease->SetMouseExitCallback(ButtonExit);
			filterRadiusIncrease->SetMouseUpCallback(ButtonEnter);
			filterRadiusIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			filterRadiusIncrease->SetActive(false);
			group.push_back(filterRadiusIncrease);
			canvas->AddUIElement(filterRadiusIncrease);

			UITextButton* filterRadiusDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
			filterRadiusDecrease->SetMouseEnterCallback(ButtonEnter);
			filterRadiusDecrease->SetMouseExitCallback(ButtonExit);
			filterRadiusDecrease->SetMouseUpCallback(ButtonEnter);
			filterRadiusDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			filterRadiusDecrease->SetActive(false);
			group.push_back(filterRadiusDecrease);
			canvas->AddUIElement(filterRadiusDecrease);

			// Strength
			oss << "Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomStrength();
			std::string strengthString = oss.str();
			oss = std::ostringstream();
			UIText* strength = new UIText(strengthString, glm::vec2(60.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			strength->SetActive(false);
			group.push_back(strength);
			canvas->AddUIElement(strength);

			UITextButton* strengthIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
			strengthIncrease->SetMouseEnterCallback(ButtonEnter);
			strengthIncrease->SetMouseExitCallback(ButtonExit);
			strengthIncrease->SetMouseUpCallback(ButtonEnter);
			strengthIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			strengthIncrease->SetActive(false);
			group.push_back(strengthIncrease);
			canvas->AddUIElement(strengthIncrease);

			UITextButton* strengthDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
			strengthDecrease->SetMouseEnterCallback(ButtonEnter);
			strengthDecrease->SetMouseExitCallback(ButtonExit);
			strengthDecrease->SetMouseUpCallback(ButtonEnter);
			strengthDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			strengthDecrease->SetActive(false);
			group.push_back(strengthDecrease);
			canvas->AddUIElement(strengthDecrease);

			// Dirt strength
			oss << "Dirt Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetAdvBloomLensDirtMaskStrength();
			std::string dirtStrengthString = oss.str();
			oss = std::ostringstream();
			UIText* dirtStrength = new UIText(dirtStrengthString, glm::vec2(60.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			dirtStrength->SetActive(false);
			group.push_back(dirtStrength);
			canvas->AddUIElement(dirtStrength);

			UITextButton* dirtStrengthIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
			dirtStrengthIncrease->SetMouseEnterCallback(ButtonEnter);
			dirtStrengthIncrease->SetMouseExitCallback(ButtonExit);
			dirtStrengthIncrease->SetMouseUpCallback(ButtonEnter);
			dirtStrengthIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			dirtStrengthIncrease->SetActive(false);
			group.push_back(dirtStrengthIncrease);
			canvas->AddUIElement(dirtStrengthIncrease);

			UITextButton* dirtStrengthDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
			dirtStrengthDecrease->SetMouseEnterCallback(ButtonEnter);
			dirtStrengthDecrease->SetMouseExitCallback(ButtonExit);
			dirtStrengthDecrease->SetMouseUpCallback(ButtonEnter);
			dirtStrengthDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			dirtStrengthDecrease->SetActive(false);
			group.push_back(dirtStrengthDecrease);
			canvas->AddUIElement(dirtStrengthDecrease);

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
			canvas->AddUIElement(ssaoParams);

			// Samples
			UIText* samples = new UIText(std::string("Samples: " + std::to_string(renderManager->GetRenderParams()->GetSSAOSamples())), glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			samples->SetActive(false);
			group.push_back(samples);
			canvas->AddUIElement(samples);

			UITextButton* samplesIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			samplesIncrease->SetMouseEnterCallback(ButtonEnter);
			samplesIncrease->SetMouseExitCallback(ButtonExit);
			samplesIncrease->SetMouseUpCallback(ButtonEnter);
			samplesIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			samplesIncrease->SetActive(false);
			group.push_back(samplesIncrease);
			canvas->AddUIElement(samplesIncrease);

			UITextButton* samplesDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			samplesDecrease->SetMouseEnterCallback(ButtonEnter);
			samplesDecrease->SetMouseExitCallback(ButtonExit);
			samplesDecrease->SetMouseUpCallback(ButtonEnter);
			samplesDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			samplesDecrease->SetActive(false);
			group.push_back(samplesDecrease);
			canvas->AddUIElement(samplesDecrease);

			// Radius
			oss << "Radius: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAOBias();
			std::string radiusString = oss.str();
			oss = std::ostringstream();
			UIText* radius = new UIText(radiusString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			radius->SetActive(false);
			group.push_back(radius);
			canvas->AddUIElement(radius);

			UITextButton* radiusIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			radiusIncrease->SetMouseEnterCallback(ButtonEnter);
			radiusIncrease->SetMouseExitCallback(ButtonExit);
			radiusIncrease->SetMouseUpCallback(ButtonEnter);
			radiusIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			radiusIncrease->SetActive(false);
			group.push_back(radiusIncrease);
			canvas->AddUIElement(radiusIncrease);

			UITextButton* radiusDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			radiusDecrease->SetMouseEnterCallback(ButtonEnter);
			radiusDecrease->SetMouseExitCallback(ButtonExit);
			radiusDecrease->SetMouseUpCallback(ButtonEnter);
			radiusDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			radiusDecrease->SetActive(false);
			group.push_back(radiusDecrease);
			canvas->AddUIElement(radiusDecrease);

			// Bias
			oss << "Bias: " << std::setprecision(4) << renderManager->GetRenderParams()->GetSSAOBias();
			std::string biasString = oss.str();
			oss = std::ostringstream();
			UIText* bias = new UIText(biasString, glm::vec2(60.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			bias->SetActive(false);
			group.push_back(bias);
			canvas->AddUIElement(bias);

			UITextButton* biasIncrease = new UITextButton(std::string("+"), glm::vec2(330.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
			biasIncrease->SetMouseEnterCallback(ButtonEnter);
			biasIncrease->SetMouseExitCallback(ButtonExit);
			biasIncrease->SetMouseUpCallback(ButtonEnter);
			biasIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			biasIncrease->SetActive(false);
			group.push_back(biasIncrease);
			canvas->AddUIElement(biasIncrease);

			UITextButton* biasDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
			biasDecrease->SetMouseEnterCallback(ButtonEnter);
			biasDecrease->SetMouseExitCallback(ButtonExit);
			biasDecrease->SetMouseUpCallback(ButtonEnter);
			biasDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			biasDecrease->SetActive(false);
			group.push_back(biasDecrease);
			canvas->AddUIElement(biasDecrease);

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
			canvas->AddUIElement(dirLightParams);

			// Colour R
			oss << "Colour R: " << std::setprecision(3) << ecs.GetComponent<ComponentLight>(directionalLightEntity)->Colour.r;
			std::string cRString = oss.str();
			oss = std::ostringstream();
			UIText* cR = new UIText(cRString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			cR->SetActive(false);
			group.push_back(cR);
			canvas->AddUIElement(cR);

			UITextButton* rIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			rIncrease->SetMouseEnterCallback(ButtonEnter);
			rIncrease->SetMouseExitCallback(ButtonExit);
			rIncrease->SetMouseUpCallback(ButtonEnter);
			rIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			rIncrease->SetActive(false);
			group.push_back(rIncrease);
			canvas->AddUIElement(rIncrease);

			UITextButton* rDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 1);
			rDecrease->SetMouseEnterCallback(ButtonEnter);
			rDecrease->SetMouseExitCallback(ButtonExit);
			rDecrease->SetMouseUpCallback(ButtonEnter);
			rDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			rDecrease->SetActive(false);
			group.push_back(rDecrease);
			canvas->AddUIElement(rDecrease);

			// Colour G
			oss << "Colour G: " << std::setprecision(3) << ecs.GetComponent<ComponentLight>(directionalLightEntity)->Colour.g;
			std::string cGString = oss.str();
			oss = std::ostringstream();
			UIText* cG = new UIText(cGString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			cG->SetActive(false);
			group.push_back(cG);
			canvas->AddUIElement(cG);

			UITextButton* gIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			gIncrease->SetMouseEnterCallback(ButtonEnter);
			gIncrease->SetMouseExitCallback(ButtonExit);
			gIncrease->SetMouseUpCallback(ButtonEnter);
			gIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			gIncrease->SetActive(false);
			group.push_back(gIncrease);
			canvas->AddUIElement(gIncrease);

			UITextButton* gDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			gDecrease->SetMouseEnterCallback(ButtonEnter);
			gDecrease->SetMouseExitCallback(ButtonExit);
			gDecrease->SetMouseUpCallback(ButtonEnter);
			gDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			gDecrease->SetActive(false);
			group.push_back(gDecrease);
			canvas->AddUIElement(gDecrease);

			// Colour B
			oss << "Colour B: " << std::setprecision(3) << ecs.GetComponent<ComponentLight>(directionalLightEntity)->Colour.b;
			std::string cBString = oss.str();
			oss = std::ostringstream();
			UIText* cB = new UIText(cBString, glm::vec2(60.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			cB->SetActive(false);
			group.push_back(cB);
			canvas->AddUIElement(cB);

			UITextButton* bIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
			bIncrease->SetMouseEnterCallback(ButtonEnter);
			bIncrease->SetMouseExitCallback(ButtonExit);
			bIncrease->SetMouseUpCallback(ButtonEnter);
			bIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			bIncrease->SetActive(false);
			group.push_back(bIncrease);
			canvas->AddUIElement(bIncrease);

			UITextButton* bDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 200.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 3);
			bDecrease->SetMouseEnterCallback(ButtonEnter);
			bDecrease->SetMouseExitCallback(ButtonExit);
			bDecrease->SetMouseUpCallback(ButtonEnter);
			bDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			bDecrease->SetActive(false);
			group.push_back(bDecrease);
			canvas->AddUIElement(bDecrease);

			// Ambient strength
			oss << "Ambient strength: " << std::setprecision(3) << ambientStrength;
			std::string ambientString = oss.str();
			oss = std::ostringstream();
			UIText* ambientStrength = new UIText(ambientString, glm::vec2(60.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			ambientStrength->SetActive(false);
			group.push_back(ambientStrength);
			canvas->AddUIElement(ambientStrength);

			UITextButton* ambientIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
			ambientIncrease->SetMouseEnterCallback(ButtonEnter);
			ambientIncrease->SetMouseExitCallback(ButtonExit);
			ambientIncrease->SetMouseUpCallback(ButtonEnter);
			ambientIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			ambientIncrease->SetActive(false);
			group.push_back(ambientIncrease);
			canvas->AddUIElement(ambientIncrease);

			UITextButton* ambientDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 240.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 4);
			ambientDecrease->SetMouseEnterCallback(ButtonEnter);
			ambientDecrease->SetMouseExitCallback(ButtonExit);
			ambientDecrease->SetMouseUpCallback(ButtonEnter);
			ambientDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			ambientDecrease->SetActive(false);
			group.push_back(ambientDecrease);
			canvas->AddUIElement(ambientDecrease);

			// Min Shadow Bias
			oss << "Min Shadow Bias: " << std::setprecision(4) << ecs.GetComponent<ComponentLight>(directionalLightEntity)->MinShadowBias;
			std::string minBiasString = oss.str();
			oss = std::ostringstream();
			UIText* minBias = new UIText(minBiasString, glm::vec2(60.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			minBias->SetActive(false);
			group.push_back(minBias);
			canvas->AddUIElement(minBias);

			UITextButton* minBiasIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
			minBiasIncrease->SetMouseEnterCallback(ButtonEnter);
			minBiasIncrease->SetMouseExitCallback(ButtonExit);
			minBiasIncrease->SetMouseUpCallback(ButtonEnter);
			minBiasIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			minBiasIncrease->SetActive(false);
			group.push_back(minBiasIncrease);
			canvas->AddUIElement(minBiasIncrease);

			UITextButton* minBiasDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 280.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 5);
			minBiasDecrease->SetMouseEnterCallback(ButtonEnter);
			minBiasDecrease->SetMouseExitCallback(ButtonExit);
			minBiasDecrease->SetMouseUpCallback(ButtonEnter);
			minBiasDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			minBiasDecrease->SetActive(false);
			group.push_back(minBiasDecrease);
			canvas->AddUIElement(minBiasDecrease);

			// Max Shadow Bias
			oss << "Max Shadow Bias: " << std::setprecision(4) << ecs.GetComponent<ComponentLight>(directionalLightEntity)->MaxShadowBias;
			std::string maxBiasString = oss.str();
			oss = std::ostringstream();
			UIText* maxBias = new UIText(maxBiasString, glm::vec2(60.0f, (float)SCR_HEIGHT - 320.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			maxBias->SetActive(false);
			group.push_back(maxBias);
			canvas->AddUIElement(maxBias);

			UITextButton* maxBiasIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 320.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 6);
			maxBiasIncrease->SetMouseEnterCallback(ButtonEnter);
			maxBiasIncrease->SetMouseExitCallback(ButtonExit);
			maxBiasIncrease->SetMouseUpCallback(ButtonEnter);
			maxBiasIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			maxBiasIncrease->SetActive(false);
			group.push_back(maxBiasIncrease);
			canvas->AddUIElement(maxBiasIncrease);

			UITextButton* maxBiasDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 320.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 6);
			maxBiasDecrease->SetMouseEnterCallback(ButtonEnter);
			maxBiasDecrease->SetMouseExitCallback(ButtonExit);
			maxBiasDecrease->SetMouseUpCallback(ButtonEnter);
			maxBiasDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			maxBiasDecrease->SetActive(false);
			group.push_back(maxBiasDecrease);
			canvas->AddUIElement(maxBiasDecrease);

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
			canvas->AddUIElement(postProcessParams);

			// Effect
			SystemRender& renderSystem = renderManager->GetRenderPipeline()->GetRenderSystem();
			std::string effectString = postProcessEffectToString[renderSystem.GetPostProcess()];
			UITextButton* effect = new UITextButton(effectString, glm::vec2(60.0f, (float)SCR_HEIGHT - 120.0f), glm::vec2(0.2f), glm::vec2(250.0f, 50.0f), font, glm::vec3(0.8f), 1);
			effect->SetMouseEnterCallback(ButtonEnter);
			effect->SetMouseExitCallback(ButtonExit);
			effect->SetMouseDownCallback(ButtonPress);
			effect->SetMouseUpCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			effect->SetActive(false);
			group.push_back(effect);
			canvas->AddUIElement(effect);

			// Post process strength
			oss << "Post Process Strength: " << std::setprecision(4) << renderManager->GetRenderParams()->GetPostProcessStrength();
			std::string postStrengthString = oss.str();
			oss = std::ostringstream();
			UIText* postStrength = new UIText(postStrengthString, glm::vec2(60.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.2f), font, glm::vec3(0.8f));
			postStrength->SetActive(false);
			group.push_back(postStrength);
			canvas->AddUIElement(postStrength);

			UITextButton* postStrengthIncrease = new UITextButton(std::string("+"), glm::vec2(400.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			postStrengthIncrease->SetMouseEnterCallback(ButtonEnter);
			postStrengthIncrease->SetMouseExitCallback(ButtonExit);
			postStrengthIncrease->SetMouseUpCallback(ButtonEnter);
			postStrengthIncrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterIncreaseOptionHold, this, std::placeholders::_1));
			postStrengthIncrease->SetActive(false);
			group.push_back(postStrengthIncrease);
			canvas->AddUIElement(postStrengthIncrease);

			UITextButton* postStrengthDecrease = new UITextButton(std::string("-"), glm::vec2(10.0f, (float)SCR_HEIGHT - 160.0f), glm::vec2(0.25f), glm::vec2(20.0f, 20.0f), font, glm::vec3(0.8f), 2);
			postStrengthDecrease->SetMouseEnterCallback(ButtonEnter);
			postStrengthDecrease->SetMouseExitCallback(ButtonExit);
			postStrengthDecrease->SetMouseUpCallback(ButtonEnter);
			postStrengthDecrease->SetMouseHoldCallback(std::bind(&PBRScene::ParameterDecreaseOptionHold, this, std::placeholders::_1));
			postStrengthDecrease->SetActive(false);
			group.push_back(postStrengthDecrease);
			canvas->AddUIElement(postStrengthDecrease);

			parameterGroups.push_back(group);
			group.clear();

#pragma endregion
		}

		Material* nonPBRMat = new Material();
		nonPBRMat->baseColour = glm::vec3(1.0f, 0.0f, 0.0f);
		nonPBRMat->specular = glm::vec3(1.0f, 0.0f, 0.0f);
		nonPBRMat->shininess = 100.0f;
		resources->AddMaterial("Non PBR Mat", nonPBRMat);

		Entity* nonPBRTest = ecs.New("NON PBR TEST");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(nonPBRTest->ID());
		transform->SetPosition(glm::vec3(-5.0f, 0.35f, 2.5f));
		ecs.AddComponent(nonPBRTest->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.GetComponent<ComponentGeometry>(nonPBRTest->ID())->ApplyMaterialToModel(nonPBRMat);

		{
			SCOPE_TIMER("PBRScene::CreateEntities::IBL");
#pragma region IBL
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

			renderManager->GetBakedData().InitialiseReflectionProbes(positions, localBounds, soiRadii, nearClips, farClips, renderSkybox, name, 1024u);
#pragma endregion
		}

	}

	void PBRScene::CreateSystems()
	{
		SCOPE_TIMER("PBRScene::CreateSystems");
		RegisterAllDefaultSystems();
	}

	void PBRScene::Update()
	{
		SCOPE_TIMER("PBRScene::Update");
		systemManager.ActionPreUpdateSystems();
		Scene::Update();
	
		float time = (float)glfwGetTime();

		Entity* spotLight = ecs.Find("Spot Light");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(spotLight->ID());
		glm::vec3 axis = transform->RotationAxis();
		//axis.x += 0.3 * Scene::dt * sin(time);
		//axis.y *= 0.3 * Scene::dt * sin(time);
		//axis.z += 0.3 * Scene::dt * cos(time);
		axis = glm::normalize(axis);
		transform->SetRotation(glm::vec3(0.0f, 1.0f, 0.0f), time * 50.0f);

		float fps = 1.0f / Scene::dt;

		float targetFPSPercentage = fps / 160.0f;
		if (targetFPSPercentage > 1.0f) { targetFPSPercentage = 1.0f; }

		glm::vec3 colour = glm::vec3(1.0f - targetFPSPercentage, 0.0f + targetFPSPercentage, 0.0f);

		Entity* canvasEntity = ecs.Find("Canvas");
		ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(canvasEntity->ID());
		dynamic_cast<UIText*>(canvas->UIElements()[1])->SetColour(colour);
		dynamic_cast<UIText*>(canvas->UIElements()[1])->SetText("FPS: " + std::to_string((int)fps));

		BVHTree* geometryBVH = collisionManager->GetBVHTree();

		const unsigned int meshCount = frustumCulling.GetTotalMeshes();
		const unsigned int visibleMeshes = frustumCulling.GetVisibleMeshes();
		const unsigned int nodeCount = geometryBVH->GetNodeCount();
		const unsigned int aabbTests = frustumCulling.GetTotalAABBTests();

		dynamic_cast<UIText*>(canvas->UIElements()[6])->SetText("Mesh count: " + std::to_string(meshCount));
		dynamic_cast<UIText*>(canvas->UIElements()[7])->SetText("     - Visible: " + std::to_string(visibleMeshes));
		dynamic_cast<UIText*>(canvas->UIElements()[8])->SetText("BVHN count: " + std::to_string(nodeCount));
		dynamic_cast<UIText*>(canvas->UIElements()[9])->SetText("AABB Tests: " + std::to_string(aabbTests));

		systemManager.ActionSystems();
	}

	void PBRScene::Render()
	{
		SCOPE_TIMER("PBRScene::Render");
		Scene::Render();
	}

	void PBRScene::Close()
	{
		std::cout << "Closing PBR scene" << std::endl;
	}

	void PBRScene::keyUp(int key)
	{
		SCOPE_TIMER("PBRScene::keyUp");
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
		if (key == GLFW_KEY_G) {
			bool renderGeometryColliders = (renderManager->GetRenderParams()->GetRenderOptions() & RENDER_GEOMETRY_COLLIDERS) != 0;
			Entity* uiCanvas = ecs.Find("Canvas");
			ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

			canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
			canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
		}
	}

	void PBRScene::keyDown(int key)
	{

	}

	void PBRScene::ChangePostProcessEffect()
	{
		SCOPE_TIMER("PBRScene::ChangePostProcessEffect");
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
}