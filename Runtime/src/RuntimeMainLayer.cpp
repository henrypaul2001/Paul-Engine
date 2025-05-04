#include "RuntimeMainLayer.h"
#include <PaulEngine.h>

RuntimeMainLayer::RuntimeMainLayer() : Layer("RuntimeMainLayer"), m_ViewportSize(1280, 720) {}

RuntimeMainLayer::~RuntimeMainLayer() {}

void RuntimeMainLayer::OnAttach()
{
	PE_PROFILE_FUNCTION();
	auto commandLineArgs = PaulEngine::Application::Get().GetSpecification().CommandLineArgs;
	if (commandLineArgs.Count > 1) {
		auto projectFilepath = commandLineArgs[1];
		if (!OpenProject(projectFilepath)) {
			PE_CORE_ERROR("Error opening project at path: '{0}'", projectFilepath);
			PaulEngine::Application::Get().Close();
		}
	}
	else {
		PE_CORE_ERROR("Missing project filepath");
		PaulEngine::Application::Get().Close();
	}

	std::vector<PaulEngine::RenderPass> renderPasses;

	PaulEngine::RenderPass scene2DPass = PaulEngine::RenderPass({}, nullptr, [](PaulEngine::RenderPassContext& passContext, PaulEngine::Ref<PaulEngine::Scene> sceneContext, PaulEngine::Ref<PaulEngine::Camera> activeCamera) {
		if (sceneContext)
		{
			PaulEngine::Camera* camera = activeCamera.get();
			PaulEngine::Entity cameraEntity = sceneContext->GetPrimaryCameraEntity();
			glm::mat4 transform = glm::mat4(1.0f);
			if (cameraEntity) {
				camera = &cameraEntity.GetComponent<PaulEngine::ComponentCamera>().Camera;
				transform = cameraEntity.GetComponent<PaulEngine::ComponentTransform>().GetTransform();
			}

			if (camera)
			{
				PaulEngine::Renderer2D::BeginScene(*camera, transform);

				{
					PE_PROFILE_SCOPE("Draw Quads");
					auto group = sceneContext->Group<PaulEngine::ComponentTransform>(entt::get<PaulEngine::Component2DSprite>);
					for (auto entityID : group) {
						auto [transform, sprite] = group.get<PaulEngine::ComponentTransform, PaulEngine::Component2DSprite>(entityID);
						if (sprite.TextureAtlas) {
							PaulEngine::Renderer2D::DrawQuad(transform.GetTransform(), sprite.TextureAtlas, sprite.SelectedSubTextureName, sprite.Colour, (int)entityID);
						}
						else if (sprite.Texture) {
							PaulEngine::Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TextureScale, sprite.Colour, (int)entityID);
						}
						else {
							PaulEngine::Renderer2D::DrawQuad(transform.GetTransform(), sprite.Colour, (int)entityID);
						}
					}
				}

				{
					PE_PROFILE_SCOPE("Draw Circles");
					auto view = sceneContext->View<PaulEngine::ComponentTransform, PaulEngine::Component2DCircle>();
					for (auto entityID : view) {
						auto [transform, circle] = view.get<PaulEngine::ComponentTransform, PaulEngine::Component2DCircle>(entityID);

						PaulEngine::Renderer2D::DrawCircle(transform.GetTransform(), circle.Colour, circle.Thickness, circle.Fade, (int)entityID);
					}
				}

				{
					PE_PROFILE_SCOPE("Draw Text");
					auto view = sceneContext->View<PaulEngine::ComponentTransform, PaulEngine::ComponentTextRenderer>();
					for (auto entityID : view) {
						auto [transform, text] = view.get<PaulEngine::ComponentTransform, PaulEngine::ComponentTextRenderer>(entityID);

						PaulEngine::Renderer2D::TextParams params;
						params.Colour = text.Colour;
						params.Kerning = text.Kerning;
						params.LineSpacing = text.LineSpacing;

						PaulEngine::Renderer2D::DrawString(text.TextString, text.Font, transform.GetTransform(), params, (int)entityID);
					}
				}

				PaulEngine::Renderer2D::EndScene();
			}
		}
	});
	PaulEngine::RenderPass scene3DPass = PaulEngine::RenderPass({}, nullptr, [](PaulEngine::RenderPassContext& passContext, PaulEngine::Ref<PaulEngine::Scene> sceneContext, PaulEngine::Ref<PaulEngine::Camera> activeCamera) {
		if (sceneContext)
		{
			PaulEngine::Camera* camera = activeCamera.get();
			PaulEngine::Entity cameraEntity = sceneContext->GetPrimaryCameraEntity();
			glm::mat4 transform = glm::mat4(1.0f);
			if (cameraEntity) {
				camera = &cameraEntity.GetComponent<PaulEngine::ComponentCamera>().Camera;
				transform = cameraEntity.GetComponent<PaulEngine::ComponentTransform>().GetTransform();
			}

			if (camera)
			{
				PaulEngine::Renderer::BeginScene(*camera, transform);

				{
					PE_PROFILE_SCOPE("Submit Mesh");
					auto view = sceneContext->View<PaulEngine::ComponentTransform, PaulEngine::ComponentMeshRenderer>();
					for (auto entityID : view) {
						auto [transform, mesh] = view.get<PaulEngine::ComponentTransform, PaulEngine::ComponentMeshRenderer>(entityID);
						PaulEngine::Renderer::SubmitDefaultCube(mesh.MaterialHandle, transform.GetTransform(), mesh.DepthState, mesh.CullState, (int)entityID);
					}
				}

				{
					PE_PROFILE_SCOPE("Submit lights");
					{
						PE_PROFILE_SCOPE("Directional lights");
						auto view = sceneContext->View<PaulEngine::ComponentTransform, PaulEngine::ComponentDirectionalLight>();
						for (auto entityID : view) {
							auto [transform, light] = view.get<PaulEngine::ComponentTransform, PaulEngine::ComponentDirectionalLight>(entityID);
							glm::mat4 transformMatrix = transform.GetTransform();
							glm::mat3 rotationMatrix = glm::mat3(transformMatrix);

							rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
							rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
							rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

							PaulEngine::Renderer::DirectionalLight lightSource;
							lightSource.Direction = glm::vec4(glm::normalize(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f)), 1.0f);
							lightSource.Diffuse = glm::vec4(light.Diffuse, 1.0f);
							lightSource.Specular = glm::vec4(light.Specular, 1.0f);
							lightSource.Ambient = glm::vec4(light.Ambient, 1.0f);

							//float shadowSize = light.ShadowMapProjectionSize;
							//
							//glm::mat4 lightView = glm::lookAt(-glm::vec3(lightSource.Direction) * light.ShadowMapCameraDistance, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
							//float aspectRatio = (float)m_ShadowWidth / (float)m_ShadowHeight;
							//float orthoLeft = -shadowSize * aspectRatio * 0.5f;
							//float orthoRight = shadowSize * aspectRatio * 0.5f;
							//float orthoBottom = -shadowSize * 0.5f;
							//float orthoTop = shadowSize * 0.5f;
							//
							//glm::mat4 lightProjection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, light.ShadowMapNearClip, light.ShadowMapFarClip);
							//lightSource.LightMatrix = lightProjection * lightView;

							PaulEngine::Renderer::SubmitDirectionalLightSource(lightSource);
						}
					}

					{
						PE_PROFILE_SCOPE("Point lights");
						auto view = sceneContext->View<PaulEngine::ComponentTransform, PaulEngine::ComponentPointLight>();
						for (auto entityID : view) {
							auto [transform, light] = view.get<PaulEngine::ComponentTransform, PaulEngine::ComponentPointLight>(entityID);
							glm::vec4 position = transform.GetTransform()[3];
							PaulEngine::Renderer::PointLight lightSource;
							lightSource.Position = position;
							lightSource.Position.w = light.Radius;
							lightSource.Diffuse = glm::vec4(light.Diffuse, 1.0f);
							lightSource.Specular = glm::vec4(light.Specular, 1.0f);
							lightSource.Ambient = glm::vec4(light.Ambient, 1.0f);
							PaulEngine::Renderer::SubmitPointLightSource(lightSource);
						}
					}

					{
						PE_PROFILE_SCOPE("Spot lights");
						auto view = sceneContext->View<PaulEngine::ComponentTransform, PaulEngine::ComponentSpotLight>();
						for (auto entityID : view) {
							auto [transform, light] = view.get<PaulEngine::ComponentTransform, PaulEngine::ComponentSpotLight>(entityID);
							glm::mat3 rotationMatrix = glm::mat3(transform.GetTransform());

							rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
							rotationMatrix[1] = glm::normalize(rotationMatrix[1]);
							rotationMatrix[2] = glm::normalize(rotationMatrix[2]);

							glm::vec3 position = transform.Position();
							glm::vec3 direction = rotationMatrix * glm::vec3(0.0f, 0.0f, -1.0f);;

							PaulEngine::Renderer::SpotLight lightSource;
							lightSource.Position = glm::vec4(position, light.Range);
							lightSource.Direction = glm::vec4(direction, glm::cos(glm::radians(light.InnerCutoff)));
							lightSource.Diffuse = glm::vec4(light.Diffuse, 1.0f);
							lightSource.Specular = glm::vec4(light.Specular, 1.0f);
							lightSource.Ambient = glm::vec4(light.Ambient, glm::cos(glm::radians(light.OuterCutoff)));
							lightSource.ShadowData = glm::vec4((float)light.CastShadows, light.ShadowMinBias, light.ShadowMaxBias, 1.0f);

							//glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
							//glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)m_ShadowWidth / (float)m_ShadowHeight, light.ShadowMapNearClip, light.ShadowMapFarClip);
							//lightSource.LightMatrix = projection * lightView;

							PaulEngine::Renderer::SubmitSpotLightSource(lightSource);
						}
					}
				}

				PaulEngine::Renderer::EndScene();
			}
		}
	});
	renderPasses.push_back(scene2DPass);
	renderPasses.push_back(scene3DPass);
	m_Renderer = PaulEngine::FrameRenderer(renderPasses);
}

void RuntimeMainLayer::OnDetach() {}

void RuntimeMainLayer::OnUpdate(const PaulEngine::Timestep timestep)
{
	PE_PROFILE_FUNCTION();
	PaulEngine::Renderer2D::ResetStats();
	PaulEngine::RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
	PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	PaulEngine::RenderCommand::Clear();

	if (m_ActiveScene) {
		m_ActiveScene->OnUpdateRuntime(timestep);
		m_Renderer.RenderFrame(m_ActiveScene, nullptr);
	}
}

void RuntimeMainLayer::OnImGuiRender()
{
	PE_PROFILE_FUNCTION();
}

void RuntimeMainLayer::OnEvent(PaulEngine::Event& e)
{
	PE_PROFILE_FUNCTION();

	PaulEngine::EventDispatcher dispatcher = PaulEngine::EventDispatcher(e);
	dispatcher.DispatchEvent<PaulEngine::WindowResizeEvent>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnWindowResize));
	dispatcher.DispatchEvent<PaulEngine::WindowCloseEvent>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnWindowClose));
	dispatcher.DispatchEvent<PaulEngine::SceneShouldChangeEvent>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnSceneShouldChange));
	dispatcher.DispatchEvent<PaulEngine::SceneChangedEvent>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnSceneChanged));
	dispatcher.DispatchEvent<PaulEngine::FrameEndEvent>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnFrameEnd));
	dispatcher.DispatchEvent<PaulEngine::KeyReleasedEvent>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnKeyUp));
}

bool RuntimeMainLayer::OnWindowClose(PaulEngine::WindowCloseEvent& e)
{
	if (m_ActiveScene) {
		m_ActiveScene->OnRuntimeStop();
	}
	return false;
}

bool RuntimeMainLayer::OnWindowResize(PaulEngine::WindowResizeEvent& e)
{
	if (m_ActiveScene) {
		m_ViewportSize = glm::ivec2(e.GetWidth(), e.GetHeight());
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
	}
	return false;
}

bool RuntimeMainLayer::OnSceneChanged(PaulEngine::SceneChangedEvent& e)
{
	PE_CORE_INFO(e);
	return false;
}

bool RuntimeMainLayer::OnSceneShouldChange(PaulEngine::SceneShouldChangeEvent& e)
{
	PE_CORE_INFO(e);
	m_SceneShouldChange = true;
	m_NextScene = e.GetSceneHandle();
	return true;
}

bool RuntimeMainLayer::OnFrameEnd(PaulEngine::FrameEndEvent& e)
{
	if (m_SceneShouldChange) {
		bool success = OpenScene(m_NextScene);
		m_SceneShouldChange = false;
		if (!success) {
			PE_CORE_WARN("Scene change unsuccessful");
		}
	}
	return false;
}

bool RuntimeMainLayer::OnKeyUp(PaulEngine::KeyReleasedEvent& e)
{
	if (e.GetKeyCode() == PE_KEY_N) {
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_LEFT_SHIFT)) {
			//PaulEngine::Application::Get().OnEvent(PaulEngine::SceneShouldChangeEvent(11922114141701769481));
			PaulEngine::Application::Get().OnEvent(PaulEngine::SceneShouldChangeEvent(2510263345310005911));
			//PaulEngine::Application::Get().OnEvent(PaulEngine::SceneShouldChangeEvent(0));
		}
	}
	return false;
}

bool RuntimeMainLayer::OpenProject(const std::filesystem::path& filepath)
{
	if (PaulEngine::Project::LoadRuntime(filepath)) {
		PaulEngine::AssetHandle startScene = PaulEngine::Project::GetActive()->GetSpecification().StartScene;
		if (startScene) {
			return OpenScene(startScene);
		}
		return false;
	}
	PE_CORE_ERROR("Failed to open project at path '{0}'", filepath.string().c_str());
	return false;
}

bool RuntimeMainLayer::OpenScene(PaulEngine::AssetHandle handle)
{
	PE_CORE_ASSERT(handle, "Invalid scene handle");
	PaulEngine::Ref<PaulEngine::RuntimeAssetManager> assetManager = PaulEngine::Project::GetActive()->GetRuntimeAssetManager();
	PaulEngine::AssetType type = assetManager->GetAssetType(handle);
	if (type != PaulEngine::AssetType::Scene) {
		PE_CORE_ERROR("Invalid asset type '{0}', '{1}' required", PaulEngine::AssetTypeToString(type), PaulEngine::AssetTypeToString(PaulEngine::AssetType::Scene));
		return false;
	}

	PaulEngine::Ref<PaulEngine::Scene> loadedScene = PaulEngine::AssetManager::GetAsset<PaulEngine::Scene>(handle);
	// TODO:
	/*
	* If the requested scene is already loaded, then all that will follow is the runtime being stopped then immediately resumed where it left off
	* This is because the scene and its assets are already loaded
	* Consider unloading the current scene and reloading it if the asset handle matches because the intention of opening the current scene again may be to reset the scene to its original state
	*/

	if (loadedScene) {
		if (m_ActiveScene) {
			m_ActiveScene->OnRuntimeStop();
		}
		m_ActiveScene = loadedScene;
		assetManager->ReleaseTempAssets();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_ActiveScene->OnRuntimeStart();
	}
	else {
		PE_CORE_ERROR("Error loading scene '{0}'", (uint64_t)handle);
		return false;
	}

	glm::highp_u32vec2 viewportSize = glm::highp_u32vec2(PaulEngine::Application::Get().GetWindow().GetWidth(), PaulEngine::Application::Get().GetWindow().GetHeight());
	m_ActiveScene->OnViewportResize(viewportSize.x, viewportSize.y);

	if (!assetManager->IsAssetLoaded(handle)) { assetManager->AddToLoadedAssets(m_ActiveScene, assetManager->GetMetadata(handle).Persistent); }

	PaulEngine::Application::Get().OnEvent(PaulEngine::SceneChangedEvent(m_ActiveScene->Handle));

	return true;
}
