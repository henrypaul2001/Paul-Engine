#include "PhysicsScene.h"
#include "GameInputManager.h"
#include "ConstraintPosition.h"
#include "ConstraintRotation.h"
#include "UIText.h"
namespace Engine {
	PhysicsScene::PhysicsScene(SceneManager* sceneManager) : Scene(sceneManager, "PhysicsScene")
	{
		ballCount = 0;
		inputManager = new GameInputManager(this);
		inputManager->SetCameraPointer(camera);
		SetupScene();
		renderManager->GetRenderParams()->SetBloomThreshold(10.0f);
		rebuildBVHOnUpdate = true;
	}

	PhysicsScene::~PhysicsScene() {}

	void PhysicsScene::ChangePostProcessEffect()
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

	void PhysicsScene::Update()
	{
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

	void PhysicsScene::Render()
	{
		Scene::Render();
	}

	void PhysicsScene::Close() {}

	void PhysicsScene::SetupScene()
	{
		// Configure GL global state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CreateSystems();
		CreateEntities();
	}

	void PhysicsScene::keyUp(int key)
	{
		if (key == GLFW_KEY_SLASH) {
			ChangePostProcessEffect();
		}
		else if (key == GLFW_KEY_KP_8) {
			Entity* baseBall = ecs.Find("Ball");
			Entity* clonedBall = ecs.Clone(baseBall->ID());
			ecs.GetComponent<ComponentTransform>(clonedBall->ID())->SetPosition(glm::vec3(6.5f, 8.5f, -20.0f));
			ecs.GetComponent<ComponentPhysics>(clonedBall->ID())->SetVelocity(glm::vec3(0.0f));

			std::cout << "Creating " << clonedBall->Name() << std::endl;
		}
		else if (key == GLFW_KEY_F) {
			physicsSystem.gravityAxis = -physicsSystem.gravityAxis;
		}
		else if (key == GLFW_KEY_KP_9) {
			Entity* torqueEntity = ecs.Find("Link 0");
			//ecs.GetComponent<ComponentPhysics>(torqueEntity->ID())->SetTorque(glm::vec3(0.0f, 2.0f, 0.0f));
			constraintManager->RemoveConstraint(constraintManager->GetConstraints().size() - 1);
		}
		else if (key == GLFW_KEY_KP_3) {
			Entity* cube = ecs.Find("Test Cube 2");
			ecs.GetComponent<ComponentPhysics>(cube->ID())->SetTorque(glm::vec3(5.0f, 0.0f, 0.0f));
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

	void PhysicsScene::keyDown(int key)
	{

	}

	void PhysicsScene::CreateEntities()
	{
		Material* textured = new Material();
		textured->baseColourMaps.push_back(resources->LoadTexture("Materials/cobble_floor/diffuse.png", TEXTURE_DIFFUSE, false));
		textured->specularMaps.push_back(resources->LoadTexture("Materials/cobble_floor/specular.png", TEXTURE_SPECULAR, false));
		textured->normalMaps.push_back(resources->LoadTexture("Materials/cobble_floor/normal.png", TEXTURE_NORMAL, false));
		textured->shininess = 5.0f;
		resources->AddMaterial("textured", textured);

		Entity* dirLight = ecs.New("Directional Light");
		ComponentLight directional = ComponentLight(DIRECTIONAL);
		directional.CastShadows = true;
		directional.Ambient = glm::vec3(0.2f, 0.2f, 0.2f);
		directional.Colour = glm::vec3(1.0f);
		directional.Specular = glm::vec3(0.0f);
		directional.Direction = glm::vec3(0.01f, -0.9f, -0.01f);
		directional.MinShadowBias = 0.0f;
		directional.MaxShadowBias = 0.003f;
		directional.DirectionalLightDistance = 20.0f;
		directional.ShadowProjectionSize = 30.0f;
		ecs.AddComponent(dirLight->ID(), directional);

		Entity* leftWall = ecs.New("Left Wall");
		ComponentTransform* transform = ecs.GetComponent<ComponentTransform>(leftWall->ID());
		transform->SetPosition(glm::vec3(-6.15f, -20.0f, -20.0f));
		ecs.AddComponent(leftWall->ID(), ComponentGeometry(MODEL_CUBE));
		transform->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		transform->SetOrientation(glm::angleAxis(-45.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		ecs.AddComponent(leftWall->ID(), ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionBox>(leftWall->ID())->IsMovedByCollisions(false);

		Entity* rightWall = ecs.New("Right Wall");
		transform = ecs.GetComponent<ComponentTransform>(rightWall->ID());
		transform->SetPosition(glm::vec3(6.15f, -20.0f, -20.0f));
		ecs.AddComponent(rightWall->ID(), ComponentGeometry(MODEL_CUBE));
		transform->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		transform->SetOrientation(glm::angleAxis(45.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
		ecs.AddComponent(rightWall->ID(), ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionBox>(rightWall->ID())->IsMovedByCollisions(false);

		Entity* backWall = ecs.New("Back Wall");
		transform = ecs.GetComponent<ComponentTransform>(backWall->ID());
		transform->SetPosition(glm::vec3(0.0f, -20.0f, -31.5f));
		ecs.AddComponent(backWall->ID(), ComponentGeometry(MODEL_CUBE));
		transform->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		transform->SetOrientation(glm::angleAxis(glm::radians(88.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		ecs.AddComponent(backWall->ID(), ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionBox>(backWall->ID())->IsMovedByCollisions(false);

		Entity* frontWall = ecs.New("Front Wall");
		transform = ecs.GetComponent<ComponentTransform>(frontWall->ID());
		transform->SetPosition(glm::vec3(0.0f, -20.0f, -8.5f));
		ecs.AddComponent(frontWall->ID(), ComponentGeometry(MODEL_CUBE));
		transform->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		transform->SetOrientation(glm::angleAxis(glm::radians(-88.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		ecs.AddComponent(frontWall->ID(), ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionBox>(frontWall->ID())->IsMovedByCollisions(false);

		Entity* physicsBall = ecs.New("Physics Ball");
		transform = ecs.GetComponent<ComponentTransform>(physicsBall->ID());
		transform->SetPosition(glm::vec3(8.0f, 30.0f, -20.0f));
		ecs.AddComponent(physicsBall->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.AddComponent(physicsBall->ID(), ComponentCollisionSphere(1.0f));
		ecs.AddComponent(physicsBall->ID(), ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5

		Entity* physicsBall2 = ecs.New("Ball");
		transform = ecs.GetComponent<ComponentTransform>(physicsBall2->ID());
		transform->SetPosition(glm::vec3(6.5f, 8.5f, -20.0f));
		ecs.AddComponent(physicsBall2->ID(), ComponentGeometry(MODEL_SPHERE));
		ecs.AddComponent(physicsBall2->ID(), ComponentCollisionSphere(1.0f));
		ecs.AddComponent(physicsBall2->ID(), ComponentPhysics(30.0f, 0.47f, 0.5f, true)); // drag coefficient of a sphere, surface area = 0.5
		ecs.GetComponent<ComponentCollisionSphere>(physicsBall2->ID())->IsMovedByCollisions(true);

		Entity* floor = ecs.New("Floor");
		transform = ecs.GetComponent<ComponentTransform>(floor->ID());
		transform->SetPosition(glm::vec3(0.0f, -2.0f, 0.0f));
		ecs.AddComponent(floor->ID(), ComponentGeometry(MODEL_CUBE));
		transform->SetScale(glm::vec3(10.0f, 1.0f, 10.0f));
		ecs.AddComponent(floor->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.GetComponent<ComponentCollisionAABB>(floor->ID())->IsMovedByCollisions(false);

		Entity* inelasticBall = ecs.New("Inelastic Ball");
		transform = ecs.GetComponent<ComponentTransform>(inelasticBall->ID());
		transform->SetPosition(glm::vec3(2.0f, 4.0f, -1.0f));
		ecs.AddComponent(inelasticBall->ID(), ComponentGeometry(MODEL_SPHERE));
		//ecs.GetComponent<ComponentGeometry>(inelasticBall->ID())->GetModel()->ApplyMaterialsToAllMesh({ textured });
		ecs.AddComponent(inelasticBall->ID(), ComponentCollisionSphere(1.0f));
		ecs.AddComponent(inelasticBall->ID(), ComponentPhysics(10.0f, 0.47f, 0.5f, 0.15f, true));

		Entity* elasticBall = ecs.New("Elastic Ball");
		transform = ecs.GetComponent<ComponentTransform>(elasticBall->ID());
		transform->SetPosition(glm::vec3(2.0f, 8.0f, -1.0f));
		ecs.AddComponent(elasticBall->ID(), ComponentGeometry(MODEL_SPHERE));
		transform->SetScale(0.5f);
		//ecs.GetComponent<ComponentGeometry>(elasticBall->ID())->GetModel()->ApplyMaterialsToAllMesh({ textured });
		ecs.AddComponent(elasticBall->ID(), ComponentCollisionSphere(1.0f));
		ecs.AddComponent(elasticBall->ID(), ComponentPhysics(10.0f, 0.47f, 0.5f, 1.0f, true));

		Entity* box = ecs.New("Box");
		transform = ecs.GetComponent<ComponentTransform>(box->ID());
		transform->SetPosition(glm::vec3(-5.0f, 10.0f, -1.0f));
		//transform->SetRotation(glm::vec3(0.5f, 0.0f, 1.0f), 45.0f);
		ecs.AddComponent(box->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.AddComponent(box->ID(), ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.AddComponent(box->ID(), ComponentPhysics(5.0f, 1.05f, 2.0f, 0.7f, true, true));

		//Entity* box2 = ecs.New("Box 2");
		//transform = ecs.GetComponent<ComponentTransform>(box2->ID());
		//transform->SetPosition(glm::vec3(-4.5f, 13.0f, -1.0f));
		//transform->SetRotation(glm::vec3(0.5f, 0.0f, 1.0f), 45.0f);
		//ecs.AddComponent(box2->ID(), ComponentGeometry(MODEL_CUBE));
		//ecs.AddComponent(box2->ID(), ComponentCollisionBox(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		//ecs.AddComponent(box2->ID(), ComponentPhysics(5.0f, 1.05f, 2.0f, 0.5f, true, true));

		// Rope bridge
		glm::vec3 linkSize = glm::vec3(2.0f, 1.0f, 1.0f);
		float linkMass = 1.0f;
		int links = 15;
		float maxConstraintDistance = 0.0f;
		float linkDistance = -3.0f;
		float bias = 0.00000005f;

		glm::vec3 startPosition = glm::vec3(-25.0f, 5.0f, 17.0f);

		Entity* bridgeStart = ecs.New("Bridge Start");
		transform = ecs.GetComponent<ComponentTransform>(bridgeStart->ID());
		transform->SetPosition(startPosition);
		transform->SetScale(linkSize);
		ecs.AddComponent(bridgeStart->ID(), ComponentGeometry(MODEL_CUBE));
		//ecs.AddComponent(bridgeStart->ID(), ComponentPhysics(10000.0f, 1.05f, 2.0f, 0.7f, false, true));
		unsigned int previous = bridgeStart->ID();

		Entity* bridgeEnd = ecs.New("Bridge End");
		transform = ecs.GetComponent<ComponentTransform>(bridgeEnd->ID());
		transform->SetPosition(startPosition + glm::vec3(0.0f, 0.0f, (links + 1) * linkDistance));
		transform->SetScale(linkSize);
		ecs.AddComponent(bridgeEnd->ID(), ComponentGeometry(MODEL_CUBE));
		//ecs.AddComponent(bridgeEnd->ID(), ComponentPhysics(10000.0f, 1.05f, 2.0f, 0.7f, false, true));
		const unsigned int bridgeEndID = bridgeEnd->ID();

		for (int i = 0; i < links; i++) {
			std::string name = std::string("Link ") + std::string(std::to_string(i));
			Entity* newLink = ecs.New(name);
			transform = ecs.GetComponent<ComponentTransform>(newLink->ID());
			transform->SetPosition(startPosition + glm::vec3(0.0f, 0.0f, (i + 1) * linkDistance));
			transform->SetScale(linkSize);
			ecs.AddComponent(newLink->ID(), ComponentPhysics(linkMass, 1.05f, 2.0f, 0.7f, true, true));
			ecs.AddComponent(newLink->ID(), ComponentGeometry(MODEL_CUBE));

			constraintManager->AddNewConstraint(new ConstraintPosition(previous, newLink->ID(), maxConstraintDistance, bias, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
			previous = newLink->ID();
		}
		constraintManager->AddNewConstraint(new ConstraintPosition(previous, bridgeEndID, maxConstraintDistance, bias, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

		Entity* testCube = ecs.New("Test Cube");
		transform = ecs.GetComponent<ComponentTransform>(testCube->ID());
		transform->SetPosition(glm::vec3(0.0f, 10.0f, 15.0f));
		ecs.AddComponent(testCube->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.AddComponent(testCube->ID(), ComponentPhysics(linkMass, 1.05f, 2.0f, 0.7f, false, true));
		//transform->SetOrientation(glm::angleAxis(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.5f)));
		const unsigned int testCubeID = testCube->ID();

		Entity* testCube2 = ecs.New("Test Cube 2");
		transform = ecs.GetComponent<ComponentTransform>(testCube2->ID());
		transform->SetPosition(glm::vec3(2.0f, 10.0f, 15.0f));
		ecs.AddComponent(testCube2->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.AddComponent(testCube2->ID(), ComponentPhysics(linkMass, 1.05f, 2.0f, 0.7f, false, true));

		constraintManager->AddNewConstraint(new ConstraintRotation(testCubeID, testCube2->ID(), glm::vec3(20.0f, 0.0f, 0.0f)));

		Entity* aabbCube = ecs.New("AABB Cube");
		transform = ecs.GetComponent<ComponentTransform>(aabbCube->ID());
		transform->SetPosition(glm::vec3(3.0f, 2.0f, 2.0f));
		ecs.AddComponent(aabbCube->ID(), ComponentGeometry(MODEL_CUBE));
		ecs.AddComponent(aabbCube->ID(), ComponentCollisionAABB(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f));
		ecs.AddComponent(aabbCube->ID(), ComponentPhysics(30.0f, 0.47f, 1.0f, 0.5f, true, true));

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
	}

	void PhysicsScene::CreateSystems()
	{
		RegisterAllDefaultSystems();
	}
}