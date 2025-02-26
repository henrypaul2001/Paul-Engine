#include "Sandbox2D.h"
#include <imgui.h>

#include <glm/ext/matrix_transform.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

Sandbox2D::Sandbox2D() : Layer("2D_Layer"), m_CameraController(1.6f / 0.9f, true, 1.0f, 1.5f) {}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach()
{
	PE_PROFILE_FUNCTION();
	m_Texture = PaulEngine::Texture2D::Create("assets/textures/awesomeFace.png");
	m_Texture2 = PaulEngine::Texture2D::Create("assets/textures/Checkerboard.png");
	m_Spritesheet = PaulEngine::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");
}

void Sandbox2D::OnDetach()
{
	PE_PROFILE_FUNCTION();
}


static PaulEngine::Timestep deltaTime = 0.0f;
void Sandbox2D::OnUpdate(const PaulEngine::Timestep timestep)
{
	PE_PROFILE_FUNCTION();
	deltaTime = timestep;
	m_CameraController.OnUpdate(timestep);
	
	{
		PE_PROFILE_SCOPE("Sandbox2D::OnUpdate::Renderer Prep");
		PaulEngine::Renderer2D::ResetStats();
		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();
	}

	{
		PE_PROFILE_SCOPE("Sandbox2D::OnUpdate::Renderer Draw");
		PaulEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());

		PaulEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f },  { 1.0f, 1.0f }, m_SquareColour, -45.0f);
		PaulEngine::Renderer2D::DrawQuad({ 2.0f, 0.0f },  { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 4.0f, 0.0f },  { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 6.0f, 0.0f },  { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 8.0f, 0.0f },  { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 10.0f, 0.0f }, { 1.0f, 1.0f }, m_Texture2);

		static float rotation = 0.0f;
		rotation += timestep * 25.0f;

		PaulEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 100.0f, 100.0f }, m_Texture, glm::vec2(10.0f), m_SquareColour, rotation);

		for (float y = -10.0f; y < 10.0f; y += 0.5f) {
			for (float x = -10.0f; x < 10.0f; x += 0.5f) {
				glm::vec4 colour = glm::vec4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.8f);
				PaulEngine::Renderer2D::DrawQuad(glm::vec2(x, y), glm::vec2(0.45f), colour);
			}
		}
		PaulEngine::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	PE_PROFILE_FUNCTION();
	const PaulEngine::Renderer2D::Statistics& stats = PaulEngine::Renderer2D::GetStats();
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
}

void Sandbox2D::OnEvent(PaulEngine::Event& e)
{
	PE_PROFILE_FUNCTION();
	m_CameraController.OnEvent(e);
}