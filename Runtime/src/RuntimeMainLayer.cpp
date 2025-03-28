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
	//dispatcher.DispatchEvent<PaulEngine::OnSceneShouldChange>(PE_BIND_EVENT_FN(RuntimeMainLayer::OnSceneShouldChange)); TODO: Create an on scene change event
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

	return true;
}
