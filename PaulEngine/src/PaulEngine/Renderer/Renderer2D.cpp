#include "pepch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"

#include "RenderCommand.h"
#include <glm/ext/matrix_transform.hpp>

namespace PaulEngine {
	struct Renderer2DStorage {
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	};

	static Renderer2DStorage* s_RenderData;

	void Renderer2D::Init()
	{
		PE_PROFILE_FUNCTION();
		s_RenderData = new Renderer2DStorage();

		// Square
		// ------
		float squareVertices[4 * 5] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f
		};

		s_RenderData->QuadVertexArray = VertexArray::Create();
		Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float2, "a_TexCoords", true }
		});
		s_RenderData->QuadVertexArray->AddVertexBuffer(squareVB);

		uint32_t square_indices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> squareIB;
		squareIB = IndexBuffer::Create(square_indices, 6);
		s_RenderData->QuadVertexArray->SetIndexBuffer(squareIB);

		s_RenderData->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		// Shader
		s_RenderData->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetUniformInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_RenderData;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		PE_PROFILE_FUNCTION();
		s_RenderData->TextureShader->Bind();
		s_RenderData->TextureShader->SetUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		s_RenderData->TextureShader->SetUniformMat4("u_ModelMatrix", glm::mat4(1.0f));
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
		PE_PROFILE_FUNCTION();
		s_RenderData->TextureShader->SetUniformFloat4("u_Colour", colour);
		s_RenderData->TextureShader->SetUniformFloat2("u_TextureScale", glm::vec2(1.0f));
		s_RenderData->WhiteTexture->Bind(0);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, { size.x, size.y, 1.0f } );
		s_RenderData->TextureShader->SetUniformMat4("u_ModelMatrix", transform);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture>& texture, const glm::vec2& textureScale, const glm::vec4& tintColour)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, textureScale, tintColour);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture, const glm::vec2& textureScale, const glm::vec4& tintColour)
	{
		PE_PROFILE_FUNCTION();
		s_RenderData->TextureShader->SetUniformFloat4("u_Colour", tintColour);
		s_RenderData->TextureShader->SetUniformFloat2("u_TextureScale", textureScale);
		texture->Bind(0);

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = glm::scale(transform, { size.x, size.y, 1.0f });
		s_RenderData->TextureShader->SetUniformMat4("u_ModelMatrix", transform);

		s_RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_RenderData->QuadVertexArray);
	}
}