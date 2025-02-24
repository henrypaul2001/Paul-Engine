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
}

void Sandbox2D::OnDetach()
{
	PE_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(const PaulEngine::Timestep timestep)
{
	PE_PROFILE_FUNCTION();
	m_CameraController.OnUpdate(timestep);
	
	{
		PE_PROFILE_SCOPE("Sandbox2D::OnUpdate::Renderer Prep");
		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();
	}

	{
		PE_PROFILE_SCOPE("Sandbox2D::OnUpdate::Renderer Draw");
		PaulEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());

		PaulEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 2.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 4.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 6.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
		PaulEngine::Renderer2D::DrawQuad({ 8.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);

		PaulEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 100.0f, 100.0f }, m_Texture, glm::vec2(10.0f), m_SquareColour);

		PaulEngine::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	PE_PROFILE_FUNCTION();
	ImGui::Begin("Colour Edit");
	ImGui::ColorPicker4("Square Colour", &m_SquareColour[0], ImGuiColorEditFlags_AlphaPreviewHalf);
	ImGui::End();
}

void Sandbox2D::OnEvent(PaulEngine::Event& e)
{
	PE_PROFILE_FUNCTION();
	m_CameraController.OnEvent(e);
}