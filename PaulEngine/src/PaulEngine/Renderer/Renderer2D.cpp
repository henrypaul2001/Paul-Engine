#include "pepch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"

#include "RenderCommand.h"
#include <glm/ext/matrix_transform.hpp>

namespace PaulEngine {
	struct QuadVertex {
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoords;
		float TextureIndex;
		glm::vec2 TextureScale;
		int EntityID;
	};

	struct Renderer2DData {
		// Per batch
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	
		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;
		
		glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;
	};

	static Renderer2DData s_RenderData;

	void Renderer2D::Init()
	{
		PE_PROFILE_FUNCTION();
		//s_RenderData = new Renderer2DData();

		s_RenderData.QuadVertexArray = VertexArray::Create();
		s_RenderData.QuadVertexBuffer = VertexBuffer::Create(s_RenderData.MaxVertices * sizeof(QuadVertex));
		s_RenderData.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float4, "a_Colour", true },
			{ ShaderDataType::Float2, "a_TexCoords", true },
			{ ShaderDataType::Float, "a_TexIndex", false },
			{ ShaderDataType::Float2, "a_TexScale", false },
			{ ShaderDataType::Int, "a_EntityID", false }
		});
		s_RenderData.QuadVertexArray->AddVertexBuffer(s_RenderData.QuadVertexBuffer);

		uint32_t* quadIndices = new uint32_t[s_RenderData.MaxIndices]; // heap allocated just in case max indices is changed to be substantially larger, which could cause a stack overflow

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_RenderData.MaxIndices; i += 6) {
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_RenderData.MaxIndices);
		s_RenderData.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_RenderData.QuadVertexBufferBase = new QuadVertex[s_RenderData.MaxVertices];

		s_RenderData.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData.WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		// Shader
		
		int samplers[s_RenderData.MaxTextureSlots];
		for (int i = 0; i < s_RenderData.MaxTextureSlots; i++) {
			samplers[i] = i;
		}
		
		s_RenderData.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_RenderData.TextureShader->Bind();
		s_RenderData.TextureShader->SetUniformIntArray("u_Textures", samplers, s_RenderData.MaxTextureSlots);

		s_RenderData.TextureSlots[0] = s_RenderData.WhiteTexture;

		s_RenderData.QuadVertexPositions[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_RenderData.QuadVertexPositions[1] = glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);
		s_RenderData.QuadVertexPositions[2] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		s_RenderData.QuadVertexPositions[3] = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
	}

	void Renderer2D::Shutdown()
	{
		//delete s_RenderData;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		PE_PROFILE_FUNCTION();
		s_RenderData.TextureShader->Bind();
		s_RenderData.TextureShader->SetUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		PE_PROFILE_FUNCTION();
		glm::mat4 viewProjection = camera.GetViewProjection();

		s_RenderData.TextureShader->Bind();
		s_RenderData.TextureShader->SetUniformMat4("u_ViewProjection", viewProjection);

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		PE_PROFILE_FUNCTION();
		s_RenderData.TextureShader->Bind();
		s_RenderData.TextureShader->SetUniformMat4("u_ViewProjection", camera.GetProjection() * glm::inverse(transform));

		StartNewBatch();
	}

	void Renderer2D::EndScene()
	{
		PE_PROFILE_FUNCTION();

		uint32_t dataSize = (uint8_t*)s_RenderData.QuadVertexBufferPtr - (uint8_t*)s_RenderData.QuadVertexBufferBase;
		s_RenderData.QuadVertexBuffer->SetData(s_RenderData.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		PE_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < s_RenderData.TextureSlotIndex; i++) {
			s_RenderData.TextureSlots[i]->Bind(i);
		}
		RenderCommand::DrawIndexed(s_RenderData.QuadVertexArray, s_RenderData.QuadIndexCount);
		s_RenderData.Stats.DrawCalls++;
	}

	void Renderer2D::StartNewBatch()
	{
		s_RenderData.QuadIndexCount = 0;
		s_RenderData.TextureSlotIndex = 1;
		s_RenderData.QuadVertexBufferPtr = s_RenderData.QuadVertexBufferBase;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.QuadIndexCount >= Renderer2DData::MaxIndices) {
			EndScene();
			StartNewBatch();
		}

		const glm::vec2 textureCoords[] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		for (int i = 0; i < 4; i++) {
			s_RenderData.QuadVertexBufferPtr->Position = transform * s_RenderData.QuadVertexPositions[i];
			s_RenderData.QuadVertexBufferPtr->Colour = colour;
			s_RenderData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_RenderData.QuadVertexBufferPtr->TextureIndex = 0;
			s_RenderData.QuadVertexBufferPtr->TextureScale = { 1.0f, 1.0f };
			s_RenderData.QuadVertexBufferPtr->EntityID = entityID;
			s_RenderData.QuadVertexBufferPtr++;
		}

		s_RenderData.QuadIndexCount += 6;

		s_RenderData.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture>& texture, const glm::vec2& textureScale, const glm::vec4& tintColour, int entityID)
	{
		PE_PROFILE_FUNCTION();
		
		if (s_RenderData.QuadIndexCount >= Renderer2DData::MaxIndices) {
			EndScene();
			StartNewBatch();
		}

		// Check if texture has already been submitted
		float textureIndex = 0.0f;
		for (int i = 1; i < s_RenderData.TextureSlotIndex; i++) {
			if (*s_RenderData.TextureSlots[i].get() == *texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			textureIndex = (float)s_RenderData.TextureSlotIndex;
			s_RenderData.TextureSlots[s_RenderData.TextureSlotIndex] = texture;
			s_RenderData.TextureSlotIndex++;
		}

		const glm::vec2 textureCoords[] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		for (int i = 0; i < 4; i++) {
			s_RenderData.QuadVertexBufferPtr->Position = transform * s_RenderData.QuadVertexPositions[i];
			s_RenderData.QuadVertexBufferPtr->Colour = tintColour;
			s_RenderData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_RenderData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_RenderData.QuadVertexBufferPtr->TextureScale = textureScale;
			s_RenderData.QuadVertexBufferPtr->EntityID = entityID;
			s_RenderData.QuadVertexBufferPtr++;
		}

		s_RenderData.QuadIndexCount += 6;

		s_RenderData.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subtexture, const glm::vec2& textureScale, const glm::vec4& tintColour, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.QuadIndexCount >= Renderer2DData::MaxIndices) {
			EndScene();
			StartNewBatch();
		}

		// Check if texture has already been submitted
		float textureIndex = 0.0f;
		for (int i = 1; i < s_RenderData.TextureSlotIndex; i++) {
			if (*s_RenderData.TextureSlots[i].get() == *subtexture->GetTexture().get()) {
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) {
			textureIndex = (float)s_RenderData.TextureSlotIndex;
			s_RenderData.TextureSlots[s_RenderData.TextureSlotIndex] = subtexture->GetTexture();
			s_RenderData.TextureSlotIndex++;
		}

		const glm::vec2* textureCoords = subtexture->GetTexCoords();

		for (int i = 0; i < 4; i++) {
			s_RenderData.QuadVertexBufferPtr->Position = transform * s_RenderData.QuadVertexPositions[i];
			s_RenderData.QuadVertexBufferPtr->Colour = tintColour;
			s_RenderData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_RenderData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_RenderData.QuadVertexBufferPtr->TextureScale = textureScale;
			s_RenderData.QuadVertexBufferPtr->EntityID = entityID;
			s_RenderData.QuadVertexBufferPtr++;
		}

		s_RenderData.QuadIndexCount += 6;

		s_RenderData.Stats.QuadCount++;
	}

	void Renderer2D::ResetStats()
	{
		s_RenderData.Stats.DrawCalls = 0;
		s_RenderData.Stats.QuadCount = 0;
	}

	const Renderer2D::Statistics& Renderer2D::GetStats()
	{
		return s_RenderData.Stats;
	}
}