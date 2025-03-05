#include "EditorLayer.h"
#include <imgui.h>

#include <glm/ext/matrix_transform.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

namespace PaulEngine {
	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_ViewportSize(1280.0f, 720.0f) {}

	EditorLayer::~EditorLayer() {}

	void EditorLayer::OnAttach()
	{
		PE_PROFILE_FUNCTION();

		FramebufferSpecification spec;
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);

		m_ActiveScene = CreateRef<Scene>();
		m_SquareEntity = m_ActiveScene->CreateEntity("Square");
		m_SquareEntity.HasComponent<ComponentTransform>();
		m_SquareEntity.AddComponent<Component2DSprite>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_CameraEntity.GetComponent<ComponentTransform>().Position = glm::vec3(0.0f, 0.0f, -1.0f);
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

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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
		}

		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
		RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		RenderCommand::Clear();

		// Update scene
		m_ActiveScene->OnUpdate(timestep);

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
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit")) { Application::Get().Close(); }
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		const Renderer2D::Statistics& stats = Renderer2D::GetStats();
		ImGui::Begin("Renderer2D");
		ImGui::SeparatorText("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Timestep (ms): %f", deltaTime.GetMilliseconds());
		ImGui::Text("FPS: %d", (int)(1.0f / deltaTime.GetSeconds()));
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportFocus = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocus || !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != glm::vec2(viewportPanelSize.x, viewportPanelSize.y)) {
			m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
		}

		uint32_t textureID = m_Framebuffer->GetColourAttachmentID();
		ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		ImGui::End();
		ImGui::PopStyleVar();

		m_SceneHierarchyPanel.OnImGuiRender();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		PE_PROFILE_FUNCTION();

		EventDispatcher dispatcher = EventDispatcher(e);
		dispatcher.DispatchEvent<KeyReleasedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnKeyUp));
	}

	bool EditorLayer::OnKeyUp(KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == PE_KEY_ESCAPE) { Application::Get().Close(); return true; }
	}
}