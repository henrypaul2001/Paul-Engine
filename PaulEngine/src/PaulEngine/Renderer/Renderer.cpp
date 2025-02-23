#include "pepch.h"
#include "Renderer.h"

#include "Renderer2D.h"

namespace PaulEngine {
	Scope<Renderer::SceneData> Renderer::m_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		PE_PROFILE_FUNCTION();
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		PE_PROFILE_FUNCTION();
		RenderCommand::SetViewport({ 0, 0 }, { width, height });
	}

	void Renderer::BeginScene(const OrthographicCamera& camera)
	{
		PE_PROFILE_FUNCTION();
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
		PE_PROFILE_FUNCTION();
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		PE_PROFILE_FUNCTION();
		shader->Bind();
		shader->SetUniformMat4("u_ModelMatrix", transform);
		shader->SetUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}