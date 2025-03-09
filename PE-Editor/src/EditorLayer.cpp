#include "EditorLayer.h"
#include <imgui.h>

#include <glm/ext/matrix_transform.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

#include <PaulEngine/Scene/SceneSerializer.h>
#include <PaulEngine/Utils/PlatformUtils.h>

#include <ImGuizmo.h>
#include <PaulEngine/Maths/Maths.h>

namespace PaulEngine {
	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_ViewportSize(1280.0f, 720.0f), m_CurrentFilepath(std::string()) {}

	EditorLayer::~EditorLayer() {}

	void EditorLayer::OnAttach()
	{
		PE_PROFILE_FUNCTION();

		FramebufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);

		m_ActiveScene = CreateRef<Scene>();

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

		//SceneSerializer serializer = SceneSerializer(m_ActiveScene);
		//serializer.DeserializeYAML("assets/scenes/Example.paul");
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

		m_Camera.OnUpdate(timestep);

		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
		RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		RenderCommand::Clear();

		// Clear entity ID attachment to -1
		m_Framebuffer->ClearColourAttachment(1, -1);

		// Update scene
		m_ActiveScene->OnUpdateOffline(timestep, m_Camera);

		ImVec2 mousePos = ImGui::GetMousePos();
		mousePos.x -= m_ViewportBounds[0].x;
		mousePos.y -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		mousePos.y = viewportSize.y - mousePos.y;

		int mouseX = (int)mousePos.x;
		int mouseY = (int)mousePos.y;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			m_HoveredEntityID = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
		}

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
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "ESC")) { Application::Get().Close(); }
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		const Renderer2D::Statistics& stats = Renderer2D::GetStats();
		ImGui::Begin("Renderer2D");
		ImGui::Text("Hovered entity: %d", m_HoveredEntityID);
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
		ImVec2 viewportOffset = ImGui::GetCursorPos(); // Includes tab bar

		m_ViewportFocus = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocus && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != glm::vec2(viewportPanelSize.x, viewportPanelSize.y)) {
			m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
		}

		uint32_t textureID = m_Framebuffer->GetColourAttachmentID();
		ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		ImVec2 windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = ImVec2(minBound.x + windowSize.x, minBound.y + windowSize.y);
		m_ViewportBounds[0] = glm::vec2(minBound.x, minBound.y);
		m_ViewportBounds[1] = glm::vec2(maxBound.x, maxBound.y);

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1) {
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

			glm::mat4 cameraView = m_Camera.GetViewMatrix();
			const glm::mat4& cameraProjection = m_Camera.GetProjection();

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

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		PE_PROFILE_FUNCTION();

		m_Camera.OnEvent(e);

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
		}
		return false;
	}

	bool EditorLayer::OnMouseUp(MouseButtonReleasedEvent& e)
	{
		if (e.GetMouseButton() == PE_MOUSE_BUTTON_LEFT && m_HoveredEntityID != -1) {
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntityID);
			return true;
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_CurrentFilepath = std::string();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Paul Engine Scene (*.paul)\0*.paul\0");

		if (!filepath.empty())
		{
			NewScene();

			SceneSerializer serializer = SceneSerializer(m_ActiveScene);
			serializer.DeserializeYAML(filepath);
			m_CurrentFilepath = filepath;
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
}