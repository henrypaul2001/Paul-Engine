#include "Sandbox2D.h"
#include <imgui.h>
#include <Platform/OpenGL/OpenGLShader.h>
#include <glm/ext/matrix_transform.hpp>
#include <PaulEngine/Renderer/Renderer2D.h>

Sandbox2D::Sandbox2D() : Layer("2D_Layer"), m_CameraController(1.6f / 0.9f, true, 1.0f, 1.5f) {}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach()
{
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(const PaulEngine::Timestep timestep)
{
	m_CameraController.OnUpdate(timestep);

	PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	PaulEngine::RenderCommand::Clear();

	PaulEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());

	PaulEngine::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
	PaulEngine::Renderer2D::DrawQuad({ 2.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
	PaulEngine::Renderer2D::DrawQuad({ 4.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
	PaulEngine::Renderer2D::DrawQuad({ 6.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);
	PaulEngine::Renderer2D::DrawQuad({ 8.0f, 0.0f }, { 1.0f, 1.0f }, m_SquareColour);

	PaulEngine::Renderer2D::EndScene();

	//auto flatColourShader = m_ShaderLibrary.Get("FlatColour");
	//std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(flatColourShader)->Bind();
	//std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(flatColourShader)->UploadUniformFloat4("u_Colour", m_SquareColour);

	PaulEngine::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Colour Edit");
	ImGui::ColorPicker4("Square Colour", &m_SquareColour[0], ImGuiColorEditFlags_AlphaPreviewHalf);
	ImGui::End();
}

void Sandbox2D::OnEvent(PaulEngine::Event& e)
{
	m_CameraController.OnEvent(e);
}