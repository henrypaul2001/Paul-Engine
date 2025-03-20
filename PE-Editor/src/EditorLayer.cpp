#include "EditorLayer.h"
#include <imgui.h>

#include <glm/ext/matrix_transform.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

#include <PaulEngine/Scene/SceneSerializer.h>
#include <PaulEngine/Utils/PlatformUtils.h>

#include <ImGuizmo.h>
#include <PaulEngine/Maths/Maths.h>

#include "PaulEngine/Renderer/Font.h"

namespace PaulEngine {
	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_ViewportSize(1280.0f, 720.0f), m_CurrentFilepath(std::string()) {
		Font font = Font("assets/fonts/Open_Sans/static/OpenSans-Regular.ttf");
	}

	EditorLayer::~EditorLayer() {}

	void EditorLayer::OnAttach()
	{
		PE_PROFILE_FUNCTION();

		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;

		m_IconPlay = Texture2D::Create("Resources/Icons/mingcute--play-fill-light.png");
		m_IconStop = Texture2D::Create("Resources/Icons/mingcute--stop-fill-light.png");
		m_IconSimulate = Texture2D::Create("Resources/Icons/mingcute--play-line-light.png");

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1) {
			auto projectFilepath = commandLineArgs[1];
			OpenProject(projectFilepath);
		}
		else {
			// TODO: Once a default project spec has been provided for NewProject(), this will be replaced with:
			// if (!OpenProject()) { NewProject(); }
			bool projectSelected = OpenProject();
			int attempts = 0;
			while (!projectSelected) {
				PE_CORE_WARN("Invalid project file");
				projectSelected = OpenProject();
				attempts++;

				if (attempts > 5) {
					Application::Get().Close();
					break;
				}
			}
		}

#if 0
		m_SquareEntity = m_ActiveScene->CreateEntity("Square");
		m_SquareEntity.HasComponent<ComponentTransform>();
		m_SquareEntity.AddComponent<Component2DSprite>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		m_SquareTwo = m_ActiveScene->CreateEntity("Square 2");
		m_SquareTwo.AddComponent<Component2DSprite>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		m_SquareTwo.GetComponent<ComponentTransform>().Position -= glm::vec3(1.5f, 0.0f, 0.0f);

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEntity.GetComponent<ComponentTransform>().Position = glm::vec3(0.0f, 0.0f, -0.5f);
		m_CameraEntity.AddComponent<ComponentCamera>(SCENE_CAMERA_ORTHOGRAPHIC);

		class CameraController : public EntityScript {
		public:
			void OnCreate() override {
				PE_INFO("Camera Controller Created!");
			}

			void OnDestroy() override {
				PE_INFO("Camera Controller Destroyed");
			}

			void OnUpdate(Timestep timestep) override {
				ComponentTransform& transform = m_Entity.GetComponent<ComponentTransform>();
				float speed = 5.0f;

				if (Input::IsKeyPressed(PE_KEY_W)) {
					transform.Position.y += speed * timestep;
				}
				if (Input::IsKeyPressed(PE_KEY_S)) {
					transform.Position.y -= speed * timestep;
				}
				if (Input::IsKeyPressed(PE_KEY_A)) {
					transform.Position.x -= speed * timestep;
				}
				if (Input::IsKeyPressed(PE_KEY_D)) {
					transform.Position.x += speed * timestep;
				}
			}
		};

		m_CameraEntity.AddComponent<ComponentNativeScript>().Bind<CameraController>();
#endif

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_Camera = EditorCamera(90.0f, 1.778f, 0.01f, 1000.0f);
	}

	void EditorLayer::OnDetach()
	{
		PE_PROFILE_FUNCTION();
	}


	static Timestep deltaTime = 0.0f;
	void EditorLayer::OnUpdate(const Timestep timestep)
	{
		PE_PROFILE_FUNCTION();
		deltaTime = timestep;

		// Resize
		const FramebufferSpecification& spec = m_Framebuffer->GetSpecification();
		if ((uint32_t)m_ViewportSize.x != spec.Width || (uint32_t)m_ViewportSize.y != spec.Height) {
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}

		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
		RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		RenderCommand::Clear();

		// Clear entity ID attachment to -1
		m_Framebuffer->ClearColourAttachment(1, -1);

		switch (m_SceneState)
		{
			case SceneState::Edit:
				m_Camera.OnUpdate(timestep, ImGuizmo::IsOver());
				m_ActiveScene->OnUpdateOffline(timestep, m_Camera);
				break;
			case SceneState::Simulate:
				m_Camera.OnUpdate(timestep, ImGuizmo::IsOver());
				m_ActiveScene->OnUpdateSimulation(timestep, m_Camera);
				break;
			case SceneState::Play:
				m_ActiveScene->OnUpdateRuntime(timestep);
				break;
		}

		ImVec2 mousePos = ImGui::GetMousePos();
		mousePos.x -= m_ViewportBounds[0].x;
		mousePos.y -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		mousePos.y = viewportSize.y - mousePos.y;

		int mouseX = (int)mousePos.x;
		int mouseY = (int)mousePos.y;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			m_HoveredEntity = Entity((entt::entity)m_Framebuffer->ReadPixel(1, mouseX, mouseY), m_ActiveScene.get());
		}
		else {
			m_HoveredEntity = Entity();
		}

		OnDebugOverlayDraw();

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();

		static bool dockSpaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen) {
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
			window_flags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen) {
			ImGui::PopStyleVar(2);
		}

		// Dockspace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWindowSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 380.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWindowSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::SeparatorText("Scene");
				if (ImGui::MenuItem("New", "LCtrl+N")) {
					NewScene();
				}
				if (ImGui::MenuItem("Open...", "LCrtl+O"))
				{
					OpenScene();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Save", "LCtrl+S")) {
					SaveSceneAs(m_CurrentFilepath);
				}
				if (ImGui::MenuItem("Save As...", "LCtrl+LShift+S")) {
					SaveSceneAs();
				}
				ImGui::SeparatorText("Project");
				ImGui::BeginDisabled();
				if (ImGui::MenuItem("New Project")) { NewProject(); }
				ImGui::EndDisabled();
				if (ImGui::MenuItem("Open Project...")) { OpenProject(); }
				if (ImGui::MenuItem("Save Project As...")) { SaveProjectAs(); }
				if (ImGui::MenuItem("Exit", "ESC")) { Application::Get().Close(); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				ImGui::Checkbox("Show colliders", &m_ShowColliders);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		const Renderer2D::Statistics& stats = Renderer2D::GetStats();
		ImGui::Begin("Renderer2D");
		std::string hoveredEntityName = "null";
		if (m_HoveredEntity.BelongsToScene(m_ActiveScene) && m_HoveredEntity) {
			hoveredEntityName = m_HoveredEntity.GetComponent<ComponentTag>().Tag;
		}
		ImGui::Text("Hovered entity: %s", hoveredEntityName.c_str());
		ImGui::SeparatorText("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Timestep (ms): %f", deltaTime.GetMilliseconds());
		ImGui::Text("FPS: %d", (int)(1.0f / deltaTime.GetSeconds()));
		ImGui::End();

		// -- Viewport --
		// --------------
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
		ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		ImVec2 viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = glm::vec2(viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y);
		m_ViewportBounds[1] = glm::vec2(viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y);

		m_ViewportFocus = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocus && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);

		uint32_t textureID = m_Framebuffer->GetColourAttachmentID();
		ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
			if (payload) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1) {
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			glm::mat4 cameraView;
			glm::mat4 cameraProjection;

			if (m_SceneState == SceneState::Play) {
				Entity runtimeCameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				Camera& camera = runtimeCameraEntity.GetComponent<ComponentCamera>().Camera;
				cameraView = glm::inverse(runtimeCameraEntity.GetComponent<ComponentTransform>().GetTransform());
				cameraProjection = camera.GetProjection();
			}
			else {
				cameraView = m_Camera.GetViewMatrix();
				cameraProjection = m_Camera.GetProjection();
			}

			// Selected entity
			ComponentTransform& transformComponent = selectedEntity.GetComponent<ComponentTransform>();
			glm::mat4 entityTransform = transformComponent.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(PE_KEY_LEFT_CONTROL);
			float snapValue = 0.25f;
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) {
				snapValue = 45.0f;
			}

			float snapValues[3] = { snapValue, snapValue, snapValue };
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::MODE::LOCAL, glm::value_ptr(entityTransform), 
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing()) {
				glm::vec3 position = glm::vec3();
				glm::vec3 rotation = glm::vec3();
				glm::vec3 scale = glm::vec3();
				Maths::DecomposeTransform(entityTransform, position, rotation, scale);
				glm::vec3 deltaRotation = rotation - transformComponent.Rotation;

				transformComponent.Position = position;
				transformComponent.Rotation += deltaRotation;
				transformComponent.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel->ImGuiRender();

		OnUIDrawToolbar();

		ImGui::End();
	}

	void EditorLayer::OnUIDrawToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colours = ImGui::GetStyle().Colors;
		auto& hovered = colours[ImGuiCol_ButtonHovered];
		auto& active = colours[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hovered.x, hovered.y, hovered.z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(active.x, active.y, active.z, 0.5f));

		ImGui::Begin("##Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetContentRegionAvail().y - 4.0f;
		{
			const char* tooltip = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? "Play" : "Stop";
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton("##switch_state", icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1))) {
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) {
					OnScenePlay();
				}
				else if (m_SceneState == SceneState::Play) {
					OnSceneStop();
				}
			}
			ImGui::SetItemTooltip(tooltip);
		}
		ImGui::SameLine();
		{
			const char* tooltip = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? "Simulate" : "Stop";
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) ? m_IconSimulate : m_IconStop;
			if (ImGui::ImageButton("##switch_state2", icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1))) {
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) {
					OnSceneSimulate();
				}
				else if (m_SceneState == SceneState::Simulate) {
					OnSceneStop();
				}
			}
			ImGui::SetItemTooltip(tooltip);
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Simulate) {
			OnSceneStop();
		}

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Play;
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play) {
			OnSceneStop();
		}

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		m_SceneState = SceneState::Simulate;
	}

	void EditorLayer::OnSceneStop()
	{
		if (m_SceneState == SceneState::Play) {
			m_ActiveScene->OnRuntimeStop();
		}
		else if (m_SceneState == SceneState::Simulate) {
			m_ActiveScene->OnSimulationStop();
		}
		else {
			return;
		}

		m_ActiveScene = m_EditorScene;
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Edit;
	}

	void EditorLayer::OnDuplicatedEntity()
	{
		if (m_SceneState == SceneState::Edit) {
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity) {
				Entity newEntity = m_ActiveScene->DuplicateEntity(selectedEntity);
				m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
			}
		}
	}

	void EditorLayer::OnDebugOverlayDraw()
	{
		PE_PROFILE_FUNCTION();
		if (m_SceneState == SceneState::Play) {
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			if (!cameraEntity) { return; }
			Renderer2D::BeginScene(cameraEntity.GetComponent<ComponentCamera>().Camera, cameraEntity.GetComponent<ComponentTransform>().GetTransform());
		}
		else {
			Renderer2D::BeginScene(m_Camera);
		}

		if (m_ShowColliders) {
			// Box colliders
			auto boxView = m_ActiveScene->GetAllEntitiesWith<ComponentTransform, ComponentBoxCollider2D>();
			for (auto entityID : boxView) {
				auto [transform, box] = boxView.get<ComponentTransform, ComponentBoxCollider2D>(entityID);

				glm::vec3 position = glm::vec3(glm::vec2(transform.Position) + box.Offset, 0.01f);
				glm::vec3 scale = transform.Scale * (glm::vec3(box.Size * 2.0f, 1.0f));
				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
				transformation = glm::rotate(transformation, transform.Rotation.z, glm::vec3(0.0, 0.0, 1.0f));
				transformation = glm::scale(transformation, scale);

				Renderer2D::SetLineWidth(0.01f);
				Renderer2D::DrawRect(transformation, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), (int)entityID);
			}
			
			// Circle colliders
			auto circleView = m_ActiveScene->GetAllEntitiesWith<ComponentTransform, ComponentCircleCollider2D>();
			for (auto entityID : circleView) {
				auto [transform, circle] = circleView.get<ComponentTransform, ComponentCircleCollider2D>(entityID);

				glm::vec3 position = glm::vec3(glm::vec2(transform.Position) + circle.Offset, 0.01f);
				glm::vec3 scale = transform.Scale * (circle.Radius * 2.0f);
				glm::mat4 transformation = glm::translate(glm::mat4(1.0f), position);
				transformation = glm::scale(transformation, scale);

				Renderer2D::DrawCircle(transformation, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.01f, 0.0f, (int)entityID);
			}
		}
		Renderer2D::Flush();
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity.BelongsToScene(m_ActiveScene) && selectedEntity) {
			ComponentTransform transformCopy = selectedEntity.GetComponent<ComponentTransform>();
			transformCopy.Position.z += 0.01f;
			Renderer2D::SetLineWidth(m_EntityOutlineThickness);
			Renderer2D::DrawRect(transformCopy.GetTransform(), m_EntityOutlineColour);
		}

		Renderer2D::EndScene();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		PE_PROFILE_FUNCTION();

		if (m_SceneState != SceneState::Play) {
			m_Camera.OnEvent(e);
		}

		EventDispatcher dispatcher = EventDispatcher(e);
		dispatcher.DispatchEvent<KeyReleasedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnKeyUp));
		dispatcher.DispatchEvent<MouseButtonReleasedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnMouseUp));
	}

	bool EditorLayer::OnKeyUp(KeyReleasedEvent& e)
	{
		bool LControl = Input::IsKeyPressed(PE_KEY_LEFT_CONTROL);
		bool LShift = Input::IsKeyPressed(PE_KEY_LEFT_SHIFT);
		switch (e.GetKeyCode()) {

			// File shortcuts
			case PE_KEY_ESCAPE:
				Application::Get().Close();
				return true;
				break;
			case PE_KEY_DELETE:
				{
					Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
					if (selectedEntity.BelongsToScene(m_ActiveScene) && selectedEntity) {
						m_SceneHierarchyPanel.SetSelectedEntity(Entity());
						m_ActiveScene->DestroyEntity(selectedEntity);
					}
					return true;
					break;
				}
			case PE_KEY_N:
				if (LControl)
				{
					NewScene();
					return true;
				}
				break;
			case PE_KEY_O:
				if (LControl)
				{
					OpenScene();
					return true;
				}
				break;
			case PE_KEY_S:
				if (LControl)
				{
					if (LShift) { SaveSceneAs(); }
					else { SaveSceneAs(m_CurrentFilepath); }
				}
				return true;
				break;

			// Gizmo shortcuts
			case PE_KEY_Q:
				m_GizmoType = -1;
				return true;
				break;
			case PE_KEY_W:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				return true;
				break;
			case PE_KEY_E:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				return true;
				break;
			case PE_KEY_R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				return true;
				break;

			// Entity shortcuts
			case PE_KEY_D:
				if (LControl) {
					OnDuplicatedEntity();
				}
				return true;
				break;
		}
		return false;
	}

	bool EditorLayer::OnMouseUp(MouseButtonReleasedEvent& e)
	{
		if (e.GetMouseButton() == PE_MOUSE_BUTTON_LEFT && CanPickEntities()) {
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			return true;
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		OnSceneStop();
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_CurrentFilepath = std::string();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Paul Engine Scene (*.paul)\0*.paul\0");

		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(std::filesystem::path filepath)
	{
		if (m_SceneState != SceneState::Edit) { OnSceneStop(); }

		std::filesystem::path extension = filepath.extension();
		if (extension != ".paul") {
			PE_CORE_WARN("Invalid scene file extension '{0}', extension '{1}' required", extension.string().c_str(), ".paul");
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer = SceneSerializer(newScene);
		if (serializer.DeserializeYAML(filepath.string())) {

			m_EditorScene = newScene;
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_EditorScene);

			m_ActiveScene = m_EditorScene;

			m_CurrentFilepath = filepath.string();
		}
	}

	void EditorLayer::SaveSceneAs(const std::string& filepath)
	{
		std::string path = filepath;
		if (path.empty()) {
			path = FileDialogs::SaveFile("Paul Engine Scene (*.paul)\0*.paul\0");
		}
		if (!path.empty()) {
			SceneSerializer serializer = SceneSerializer(m_ActiveScene);
			serializer.SerializeYAML(path);
			m_CurrentFilepath = path;
		}
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	bool EditorLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Paul Engine Project (*.pproj)\0*.pproj\0");

		if (!filepath.empty())
		{
			OpenProject(filepath);
			return true;
		}
		else {
			return false;
		}
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path)) {
			std::filesystem::path startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetSpecification().StartScenePath);
			OpenScene(startScenePath);
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
		}
	}

	void EditorLayer::SaveProjectAs()
	{
		std::string path = FileDialogs::SaveFile("Paul Engine Project (*.pproj)\0*.pproj\0");
		if (!path.empty()) {
			Project::SaveActive(path);
		}
	}

	bool EditorLayer::CanPickEntities()
	{
		return m_ViewportHovered && !ImGuizmo::IsOver() && !m_Camera.IsMoving();
	}
}