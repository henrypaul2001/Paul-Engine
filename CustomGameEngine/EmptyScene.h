#pragma once
#include "Scene.h"
#include "UIText.h"

namespace Engine
{
	class EmptyScene : public Scene
	{
	public:
		EmptyScene(SceneManager* sceneManager);
		~EmptyScene() {}

		void Update() override {
			systemManager.ActionPreUpdateSystems();
			Scene::Update();

			float time = (float)glfwGetTime();
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
		void Render() override {
			Scene::Render();
		}
		void Close() override {}
		void SetupScene() override {
			const char* envMapPath = "Textures/Environment Maps/sky.hdr";
			resources->LoadHDREnvironmentMap(envMapPath, true);
			renderManager->SetEnvironmentMap(envMapPath);
			renderManager->GetRenderParams()->EnableRenderOptions(RENDER_IBL | RENDER_ENVIRONMENT_MAP);

			renderManager->GetRenderParams()->EnableRenderOptions(RENDER_ADVANCED_BLOOM | RENDER_ADVANCED_BLOOM_LENS_DIRT);

			resources->LoadTexture("Textures/LensEffects/dirtmask.jpg", TEXTURE_DIFFUSE, false);
			renderManager->SetAdvBloomLensDirtTexture("Textures/LensEffects/dirtmask.jpg");

			rebuildBVHOnUpdate = true;

			Entity* dirLight = ecs.New("Directional Light");
			ecs.AddComponent(dirLight->ID(), ComponentLight(DIRECTIONAL));

			Entity* defaultCube = ecs.New("Cube");
			ecs.AddComponent(defaultCube->ID(), ComponentGeometry(MODEL_CUBE, true));

#pragma region UI
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
#pragma endregion
			RegisterAllDefaultSystems();
		}

		void keyUp(int key) override {
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
		void keyDown(int key) override {}
	};
}