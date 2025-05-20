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
		//m_Renderer.RenderFrame(m_ActiveScene, nullptr);
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

/*

PE_CORE_ASSERT(handle, "Invalid scene handle");
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		AssetType type = assetManager->GetAssetType(handle);
		if (type != AssetType::Scene) {
			PE_CORE_ERROR("Invalid asset type '{0}', '{1}' required", AssetTypeToString(type), AssetTypeToString(AssetType::Scene));
			return;
		}
		if (m_SceneState != SceneState::Edit) { OnSceneStop(); }

		if (m_ActiveSceneHandle != 0) {
			//Project::GetActive()->GetEditorAssetManager()->UnloadAsset(m_ActiveSceneHandle);
			assetManager->ReleaseTempAssets();
		}

		Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(handle);
		Ref<Scene> newScene = Scene::Copy(readOnlyScene);

		m_EditorScene = newScene;
		m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);

		m_ActiveScene = m_EditorScene;
		m_ActiveSceneHandle = handle;

		m_CurrentFilepath = assetManager->GetMetadata(handle).FilePath;

		Application::Get().OnEvent(SceneChangedEvent(m_ActiveSceneHandle));
*/

bool RuntimeMainLayer::OpenScene(PaulEngine::AssetHandle handle)
{
	PE_CORE_ASSERT(handle, "Invalid scene handle");
	PaulEngine::Ref<PaulEngine::RuntimeAssetManager> assetManager = PaulEngine::Project::GetActive()->GetRuntimeAssetManager();
	PaulEngine::AssetType type = assetManager->GetAssetType(handle);
	if (type != PaulEngine::AssetType::Scene) {
		PE_CORE_ERROR("Invalid asset type '{0}', '{1}' required", PaulEngine::AssetTypeToString(type), PaulEngine::AssetTypeToString(PaulEngine::AssetType::Scene));
		return false;
	}

	if (PaulEngine::AssetManager::IsAssetHandleValid(handle))
	{
		assetManager->ReleaseTempAssets();
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
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_ActiveScene->OnRuntimeStart();
	}
	else {
		PE_CORE_ERROR("Error loading scene '{0}'", (uint64_t)handle);
		return false;
	}

	glm::highp_u32vec2 viewportSize = glm::highp_u32vec2(PaulEngine::Application::Get().GetWindow().GetWidth(), PaulEngine::Application::Get().GetWindow().GetHeight());
	m_ActiveScene->OnViewportResize(viewportSize.x, viewportSize.y);

	PaulEngine::Application::Get().OnEvent(PaulEngine::SceneChangedEvent(m_ActiveScene->Handle));

	return true;
}
