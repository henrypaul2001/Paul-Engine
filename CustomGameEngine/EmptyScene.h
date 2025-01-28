#pragma once
#include "Scene.h"
#include "NavigationGrid.h"
#include "UIText.h"

namespace Engine
{
	struct TestComponentA {
		float x, y, z;
	};

	struct TestComponentB {
		float velocityX, velocityY, velocityZ;
	};

	struct TestComponentC {
		float c;
	};

	struct TestComponentD {
		bool d;
	};

	struct TestComponentE {
		long e;
	};

	struct TestComponentF {
		long long f;
	};

	static void TestSystem(const unsigned int entityID, ComponentTransform& transform, ComponentPhysics& physics) {
		std::cout << "TEST_SYSTEM: EntityID = " << entityID << std::endl;
	}
	static void TestAfterAction() {
		std::cout << "TEST_SYSTEM: After action" << std::endl;
	}

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

			EntityNew* canvasEntity = ecs.Find("Canvas");
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
			SkeletalAnimation* vampireDanceAnim = ResourceManager::GetInstance()->LoadAnimation("Models/vampire/dancing_vampire.dae");

			navGrid = NavigationGrid("Data/NavigationGrid/TestGrid1.txt");
			AudioFile* campfireCrackling = ResourceManager::GetInstance()->LoadAudio("Audio/campfire.wav", 1.0f, 0.0f, 2.0f);

			// All components entity
			EntityNew* allComponents = ecs.New("All Components");
			ecs.AddComponent(allComponents->ID(), ComponentPhysics());
			ecs.AddComponent(allComponents->ID(), ComponentAnimator(vampireDanceAnim));
			ecs.AddComponent(allComponents->ID(), ComponentAudioSource(campfireCrackling));
			ecs.AddComponent(allComponents->ID(), ComponentCollisionSphere(1.0f));
			//ecs.AddComponent(allComponents->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
			//ecs.AddComponent(allComponents->ID(), ComponentCollisionBox(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
			//ecs.AddComponent(allComponents->ID(), ComponentGeometry("Models/PBR/brass_goblet/brass_goblet.obj", true));
			ecs.AddComponent(allComponents->ID(), ComponentGeometry("Models/vampire/dancing_vampire.dae", false));
			//ecs.AddComponent(allComponents->ID(), ComponentGeometry(MODEL_SPHERE));
			ecs.AddComponent(allComponents->ID(), ComponentLight(SPOT));
			ecs.AddComponent(allComponents->ID(), ComponentParticleGenerator(ResourceManager::GetInstance()->LoadTexture("Textures/Particles/flame.png", TEXTURE_DIFFUSE, false)));
			ecs.AddComponent(allComponents->ID(), ComponentPathfinder(&navGrid));
			ecs.AddComponent(allComponents->ID(), ComponentStateController());
			ecs.GetComponent<ComponentTransform>(allComponents->ID())->SetScale(1.0f);
			ecs.GetComponent<ComponentTransform>(allComponents->ID())->SetPosition(glm::vec3(0.0f, 20.0f, 0.0f));

			EntityNew* floor = ecs.New("Floor");
			ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE));
			ComponentCollisionAABB collider = ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
			collider.IsMovedByCollisions(false);
			ecs.AddComponent(floor->ID(), collider);
			ComponentTransform* floorTransform = ecs.GetComponent<ComponentTransform>(floor->ID());
			floorTransform->SetPosition(glm::vec3(0.0f, -5.0f, 0.0f));
			floorTransform->SetScale(glm::vec3(10.0f, 0.5f, 10.0f));

			for (int i = 0; i < 50; i++) {
				ecs.New("Test");
			}
			ecs.New("Hello");

			const EntityNew* missingEntity = ecs.Find("I don't exist");
			const EntityNew* foundEntity = ecs.Find("Hello");
			const EntityNew* duplicateFoundEntity = ecs.Find("Test (10)");

			EntityNew* findTen = ecs.Find(10);

			ecs.Delete(*findTen);
			ecs.Delete(*ecs.Find(5));

			EntityNew* deletedTen = ecs.Find("Test (10)");
			EntityNew* deletedFive = ecs.Find("Test (5)");

			ecs.New("After delete");

			bool equalsFalse = ecs.Delete("I don't exist");
			bool equalsTrue = ecs.Delete("After delete");

			ecs.RegisterComponentType<TestComponentA>();
			ecs.RegisterComponentType<TestComponentB>();

			ecs.AddComponent(0, TestComponentA());
			ecs.AddComponent(42, TestComponentA());
			ecs.AddComponent(0, TestComponentB());

			bool hasA = ecs.HasComponent<TestComponentA>(0);
			bool doesntHaveA = ecs.HasComponent<TestComponentA>(1);
			bool hasB = ecs.HasComponent<TestComponentB>(0);

			std::bitset<MAX_COMPONENTS> testMask = ecs.CreateMask<TestComponentB>();

			ecs.RemoveComponent<TestComponentA, TestComponentB>(0);
			ecs.RemoveComponent<TestComponentA>(42);

			//ecs.AddComponent(2, TestComponentA());
			ecs.AddComponent(0, TestComponentA());
			ecs.AddComponent(2, TestComponentB());

			TestComponentB* bComponent = ecs.GetComponent<TestComponentB>(2);
			bComponent->velocityX = 10.0f;
			TestComponentA* nullpointer = ecs.GetComponent<TestComponentA>(2);

			ecs.Delete(2);

			std::cout << bComponent->velocityX << std::endl;
		
			EntityNew* clone_base = ecs.New("Clone base");
			TestComponentB b;
			b.velocityX = -2.5f;
			b.velocityY = 5.0f;
			ecs.AddComponent(clone_base->ID(), TestComponentA());
			ecs.AddComponent(clone_base->ID(), b);

			EntityNew* cloned = ecs.Clone(clone_base->ID());

			for (int i = 0; i < 50; i++) {
				EntityNew* entity = ecs.New("ViewTest");
				TestComponentA a;
				a.x = i;
				ecs.AddComponent(entity->ID(), a);
				if (i % 2 == 0) {
					TestComponentB b;
					b.velocityY = i;
					ecs.AddComponent(entity->ID(), b);
				}
				if (i % 3 == 0) {
					TestComponentC c;
					c.c = i;
					ecs.AddComponent(entity->ID(), c);
				}
				if (i % 4 == 0) {
					TestComponentF f;
					f.f = i;
					ecs.AddComponent(entity->ID(), f);
					ecs.AddComponent(entity->ID(), ComponentPhysics());
				}
			}

			View<TestComponentA, TestComponentB> testView = ecs.View<TestComponentA, TestComponentB>();
			testView.ForEach([](const unsigned int entityID, TestComponentA& a, TestComponentB& b) {
				unsigned int id = entityID;
				float x = a.x;
				float velocityY = b.velocityY;
			});

			View<TestComponentF> fView = ecs.View<TestComponentF>();
			fView.ForEach([](const unsigned int entityID, TestComponentF& f) {
				unsigned int id = entityID;
				if (id == 10) {
					f.f = 3000.0;
				}
			});

			std::cout << ecs.GetComponent<TestComponentF>(10)->f << std::endl;

			EntityNew* transformTest = ecs.New("Transform Test");
			ecs.AddComponent(transformTest->ID(), ComponentTransform(&ecs, 10.0f, 1.0f, -5.0f));
			//ecs.RemoveComponent<ComponentTransform>(transformTest->ID()); // build error, cannot remove transform component

			// Transform children
			for (int i = 0; i < 20; i++) {
				EntityNew* child = ecs.New("Transform Test");
				ecs.AddComponent(child->ID(), ComponentTransform(&ecs, 10.0f + i, 1.0f + i, -5.0f - i));
				ecs.GetComponent<ComponentTransform>(transformTest->ID())->AddChild(child->ID());
			}

			ComponentTransform* transformChild = ecs.GetComponent<ComponentTransform>(ecs.GetComponent<ComponentTransform>(transformTest->ID())->FindChildWithName("Transform Test (3)")->ID());

			EntityNew* geometryTest = ecs.New("Geometry Test");
			ecs.GetComponent<ComponentTransform>(geometryTest->ID())->SetPosition(glm::vec3(0.0f, 0.0f, -2.5f));
			ecs.AddComponent(geometryTest->ID(), ComponentGeometry(MODEL_CUBE));

			EntityNew* dirLight = ecs.New("Dir Light");
			ecs.GetComponent<ComponentTransform>(dirLight->ID())->SetPosition(glm::vec3(10.0f, 10.0f, 10.0f));
			ecs.AddComponent(dirLight->ID(), ComponentLight(DIRECTIONAL));

#pragma region UI
			TextFont* font = ResourceManager::GetInstance()->LoadTextFont("Fonts/arial.ttf");
			EntityNew* uiCanvas = ecs.New("Canvas");
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
				EntityNew* uiCanvas = ecs.Find("Canvas");
				ComponentUICanvas* canvas = ecs.GetComponent<ComponentUICanvas>(uiCanvas->ID());

				canvas->UIElements()[5]->SetActive(!renderGeometryColliders);
				canvas->UIElements()[6]->SetActive(!renderGeometryColliders);
				canvas->UIElements()[7]->SetActive(!renderGeometryColliders);
				canvas->UIElements()[8]->SetActive(!renderGeometryColliders);
				canvas->UIElements()[9]->SetActive(!renderGeometryColliders);
			}
		}
		void keyDown(int key) override {}

	private:
		NavigationGrid navGrid;
	};
}