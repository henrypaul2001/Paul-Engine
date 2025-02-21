#include "pepch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include <Platform/OpenGL/OpenGLShader.h>
#include "RenderCommand.h"
#include <glm/ext/matrix_transform.hpp>
namespace PaulEngine {
	struct Renderer2DStorage {
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColourShader;
	};

	static Renderer2DStorage* s_RenderData;

	void Renderer2D::Init()
	{
		s_RenderData = new Renderer2DStorage();

		// Square
		// ------
		float squareVertices[4 * 3] = {
			-0.5f, -0.5f, 0.0f,
			0.5, -0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f
		};

		s_RenderData->QuadVertexArray = VertexArray::Create();
		Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false }
			});
		s_RenderData->QuadVertexArray->AddVertexBuffer(squareVB);

		uint32_t square_indices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(square_indices, 6);
		s_RenderData->QuadVertexArray->SetIndexBuffer(squareIB);

		// Shader
		s_RenderData->FlatColourShader = Shader::Create("assets/shaders/FlatColour.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_RenderData;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_RenderData->FlatColourShader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_RenderData->FlatColourShader)->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(s_RenderData->FlatColourShader)->UploadUniformMat4("u_ModelMatrix", glm::mat4(1.0f));
	}

	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& colour)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, colour);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour)
	{
		s_RenderData->FlatColourShader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(s_RenderData->FlatColourShader)->UploadUniformFloat4("u_Colour", colour);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, { size.x, size.y, 1.0f } );
		std::dynamic_pointer_cast<OpenGLShader>(s_RenderData->FlatColourShader)->UploadUniformMat4("u_ModelMatrix", transform);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}
}