#include "EditorLayer.h"
#include <imgui.h>

#include <glm/ext/matrix_transform.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

namespace PaulEngine {
	static const char* s_MapTiles =
		"WWWWWWWWWWWWWWWWWWWWWWWW"
		"WWWWWDDDDDDDDDDWWWWWWWWW"
		"WWWDDDDDDDDDDDDDDDWWWWWW"
		"WWDDDDDDDDDDDDDDDDDDWWWW"
		"WDDDDDWWWDDDDDDDDDDDWWWW"
		"WDDDDDWWWDDDDDDDDDDDDWWW"
		"WWDDDDDDDDDDDDDDDDDDDWWW"
		"WWWWDDDDDDDDDDDDDDDDDWWc"
		"WWWWWWWWDDDDDDDDDDWWWWWW"
		"WWWWWWWWWWDDDDWWWWWWWWWW"
		"WWWWWWWWWWWWWWWWWWWWWWWW";

	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_CameraController(1.6f / 0.9f, true, 1.0f, 1.5f) {}

	EditorLayer::~EditorLayer() {}

	void EditorLayer::OnAttach()
	{
		PE_PROFILE_FUNCTION();
		m_Texture = Texture2D::Create("assets/textures/awesomeFace.png");
		m_Texture2 = Texture2D::Create("assets/textures/Checkerboard.png");

		m_Spritesheet = Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");
		m_TextureStairs = SubTexture2D::CreateFromCoords(m_Spritesheet, { 7, 6 }, { 128.0f, 128.0f });
		m_TextureBarrel = SubTexture2D::CreateFromCoords(m_Spritesheet, { 8, 2 }, { 128.0f, 128.0f });
		m_TextureTree = SubTexture2D::CreateFromCoords(m_Spritesheet, { 2, 1 }, { 128.0f, 128.0f }, { 1, 2 });
		m_TextureInvalid = SubTexture2D::CreateFromCoords(m_Texture2, { 0, 0 }, { m_Texture2->GetWidth(), m_Texture2->GetHeight() });

		m_MapWidth = 24;
		m_MapHeight = strlen(s_MapTiles) / m_MapWidth;
		m_TextureMap['D'] = SubTexture2D::CreateFromCoords(m_Spritesheet, { 6, 11 }, { 128.0f, 128.0f });
		m_TextureMap['W'] = SubTexture2D::CreateFromCoords(m_Spritesheet, { 11, 11 }, { 128.0f, 128.0f });

		FramebufferSpecification spec;
		spec.Width = 1280;
		spec.Height = 720;
		m_Framebuffer = Framebuffer::Create(spec);
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
		m_CameraController.OnUpdate(timestep);

		{
			PE_PROFILE_SCOPE("Sandbox2D::OnUpdate::Renderer Prep");
			Renderer2D::ResetStats();
			m_Framebuffer->Bind();
			RenderCommand::SetViewport({ 0.0f, 0.0f }, glm::ivec2((glm::ivec2)m_ViewportSize));
			RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			RenderCommand::Clear();
		}

		{
			PE_PROFILE_SCOPE("Sandbox2D::OnUpdate::Renderer Draw");

#if 0
			{
				Renderer2D::BeginScene(m_CameraController.GetCamera());

				Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour, -45.0f);
				Renderer2D::DrawQuad({ 2.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
				Renderer2D::DrawQuad({ 4.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
				Renderer2D::DrawQuad({ 6.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
				Renderer2D::DrawQuad({ 8.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
				Renderer2D::DrawQuad({ 10.0f, 0.0f }, { 1.0f, 1.0f }, m_Texture2);

				static float rotation = 0.0f;
				rotation += timestep * 25.0f;

				Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 100.0f, 100.0f }, m_Texture, glm::vec2(10.0f), m_SquareColour, rotation);

				for (float y = -10.0f; y < 10.0f; y += 0.5f) {
					for (float x = -10.0f; x < 10.0f; x += 0.5f) {
						glm::vec4 colour = glm::vec4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.8f);
						Renderer2D::DrawQuad(glm::vec2(x, y), glm::vec2(0.45f), colour);
					}
				}
				Renderer2D::EndScene();
			}
#endif

#if 1
			{
				Renderer2D::BeginScene(m_CameraController.GetCamera());

				for (uint32_t y = 0; y < m_MapHeight; y++) {
					for (uint32_t x = 0; x < m_MapWidth; x++) {
						char tile = s_MapTiles[x + y * m_MapWidth];
						Ref<SubTexture2D> texture;

						std::unordered_map<char, Ref<SubTexture2D>>::iterator it = m_TextureMap.find(tile);
						if (it != m_TextureMap.end()) { texture = it->second; }
						else { texture = m_TextureInvalid; }

						Renderer2D::DrawQuad({ x - m_MapWidth / 2.0f, m_MapHeight - y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, texture);
					}
				}

				Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_TextureStairs);
				Renderer2D::DrawQuad({ 0.0f, 1.0f }, { 1.0f, 1.0f }, m_TextureBarrel);
				Renderer2D::DrawQuad({ -1.5f, 0.5f }, { 1.0f, 2.0f }, m_TextureTree);
				Renderer2D::EndScene();
				m_Framebuffer->Unbind();
			}
#endif
		}
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

		ImGui::SeparatorText("Edit:");
		ImGui::ColorPicker4("Square Colour", &m_SquareColour[0], ImGuiColorEditFlags_AlphaPreviewHalf);
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != glm::vec2(viewportPanelSize.x, viewportPanelSize.y)) {
			// Viewport panel has changed, resize framebuffer
			m_ViewportSize = glm::vec2(viewportPanelSize.x, viewportPanelSize.y);
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.ResizeBounds(m_ViewportSize.x, m_ViewportSize.y);
		}
		uint32_t textureID = m_Framebuffer->GetColourAttachmentID();
		ImGui::Image(textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		PE_PROFILE_FUNCTION();
		m_CameraController.OnEvent(e);

		EventDispatcher dispatcher = EventDispatcher(e);
		dispatcher.DispatchEvent<KeyReleasedEvent>(PE_BIND_EVENT_FN(EditorLayer::OnKeyUp));
	}

	bool EditorLayer::OnKeyUp(KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == PE_KEY_ESCAPE) { Application::Get().Close(); return true; }
	}
}