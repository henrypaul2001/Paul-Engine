#include "Sandbox2D.h"
#include <imgui.h>
#include <Platform/OpenGL/OpenGLShader.h>
#include <glm/ext/matrix_transform.hpp>

Sandbox2D::Sandbox2D() : Layer("2D_Layer"), m_CameraController(1.6f / 0.9f, true, 1.0f, 1.5f) {}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach()
{
	// Square
	// ------
	float squareVertices[4 * 3] = {
		-0.5f, -0.5f, 0.0f,
		0.5, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};

	m_SquareVertexArray = PaulEngine::VertexArray::Create();
	PaulEngine::Ref<PaulEngine::VertexBuffer> squareVB;
	squareVB = PaulEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
	squareVB->SetLayout({
		{ PaulEngine::ShaderDataType::Float3, "a_Position", false }
		});
	m_SquareVertexArray->AddVertexBuffer(squareVB);

	uint32_t square_indices[6] = { 0, 1, 2, 2, 3, 0 };
	PaulEngine::Ref<PaulEngine::IndexBuffer> squareIB;
	squareIB = PaulEngine::IndexBuffer::Create(square_indices, 6);
	m_SquareVertexArray->SetIndexBuffer(squareIB);

	// Shader
	m_ShaderLibrary.Load("assets/shaders/FlatColour.glsl");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(const PaulEngine::Timestep timestep)
{
	m_CameraController.OnUpdate(timestep);

	PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	PaulEngine::RenderCommand::Clear();

	PaulEngine::Renderer::BeginScene(m_CameraController.GetCamera());

	auto flatColourShader = m_ShaderLibrary.Get("FlatColour");
	std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(flatColourShader)->Bind();
	std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(flatColourShader)->UploadUniformFloat4("u_Colour", m_SquareColour);

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
	for (int y = 0; y < 20; y++) {
		for (int x = 0; x < 20; x++) {
			glm::vec3 pos = glm::vec3(x * 0.11f, y * 0.11f, 0.0f);
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
			PaulEngine::Renderer::Submit(flatColourShader, m_SquareVertexArray, transform);
		}
	}

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