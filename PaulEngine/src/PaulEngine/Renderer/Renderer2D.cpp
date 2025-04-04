#include "pepch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "UniformBuffer.h"

#include "PaulEngine/Asset/AssetManager.h"

#include "RenderCommand.h"
#include <glm/ext/matrix_transform.hpp>

#include "MSDFData.h"
#include "TextureAtlas2D.h"

namespace PaulEngine {
	struct QuadVertex {
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoords;
		float TextureIndex;
		glm::vec2 TextureScale;
		int EntityID;
	};

	struct CircleVertex {
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Colour;
		float Thickness;
		float Fade;
		int EntityID;
	};

	struct LineVertex {
		glm::vec3 Position;
		glm::vec4 Colour;
		int EntityID;
	};

	struct TextVertex {
		glm::vec3 Position;
		glm::vec4 Colour;
		glm::vec2 TexCoords;

		// Background colour for outline

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
		Ref<Shader> QuadShader;
		Ref<Texture2D> WhiteTexture;

		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;

		Ref<VertexArray> TextVertexArray;
		Ref<VertexBuffer> TextVertexBuffer;
		Ref<Shader> TextShader;

		float LineWidth = 2.0f;
	
		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		uint32_t TextIndexCount = 0;
		TextVertex* TextVertexBufferBase = nullptr;
		TextVertex* TextVertexBufferPtr = nullptr;

		std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;
		
		Ref<Texture2D> FontAtlasTexture;

		glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
	};

	static Renderer2DData s_RenderData;

	void Renderer2D::Init()
	{
		PE_PROFILE_FUNCTION();
		//s_RenderData = new Renderer2DData();

		// -- Quad --
		// ----------
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

		// - Circle -
		// ----------

		s_RenderData.CircleVertexArray = VertexArray::Create();
		s_RenderData.CircleVertexBuffer = VertexBuffer::Create(s_RenderData.MaxVertices * sizeof(CircleVertex));
		s_RenderData.CircleVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float3, "a_LocalPosition", false },
			{ ShaderDataType::Float4, "a_Colour", true },
			{ ShaderDataType::Float, "a_Thickness", true },
			{ ShaderDataType::Float, "a_Fade", true },
			{ ShaderDataType::Int, "a_EntityID", false }
		});
		s_RenderData.CircleVertexArray->AddVertexBuffer(s_RenderData.CircleVertexBuffer);
		s_RenderData.CircleVertexArray->SetIndexBuffer(quadIB); // Re-use quadIB
		s_RenderData.CircleVertexBufferBase = new CircleVertex[s_RenderData.MaxVertices];

		// -- Line --
		// ----------
		s_RenderData.LineVertexArray = VertexArray::Create();
		s_RenderData.LineVertexBuffer = VertexBuffer::Create(s_RenderData.MaxVertices * sizeof(LineVertex));
		s_RenderData.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float4, "a_Colour", true },
			{ ShaderDataType::Int, "a_EntityID", false }
		});
		s_RenderData.LineVertexArray->AddVertexBuffer(s_RenderData.LineVertexBuffer);
		s_RenderData.LineVertexBufferBase = new LineVertex[s_RenderData.MaxVertices];

		uint32_t whiteTextureData = 0xffffffff;
		s_RenderData.WhiteTexture = Texture2D::Create(TextureSpecification(), Buffer(&whiteTextureData, sizeof(uint32_t)));

		// -- Text --
		// ----------
		s_RenderData.TextVertexArray = VertexArray::Create();
		s_RenderData.TextVertexBuffer = VertexBuffer::Create(s_RenderData.MaxVertices * sizeof(TextVertex));
		s_RenderData.TextVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float4, "a_Colour", true },
			{ ShaderDataType::Float2, "a_TexCoords", true },
			{ ShaderDataType::Int, "a_EntityID", false }
		});
		s_RenderData.TextVertexArray->AddVertexBuffer(s_RenderData.TextVertexBuffer);
		s_RenderData.TextVertexArray->SetIndexBuffer(quadIB);
		s_RenderData.TextVertexBufferBase = new TextVertex[s_RenderData.MaxVertices];

		// Shader
		
		int samplers[s_RenderData.MaxTextureSlots];
		for (int i = 0; i < s_RenderData.MaxTextureSlots; i++) {
			samplers[i] = i;
		}
		
		s_RenderData.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
		s_RenderData.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
		s_RenderData.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
		s_RenderData.TextShader = Shader::Create("assets/shaders/Renderer2D_Text.glsl");

		s_RenderData.QuadShader->Bind();
		s_RenderData.QuadShader->SetUniformIntArray("u_Textures", samplers, s_RenderData.MaxTextureSlots);

		s_RenderData.TextureSlots[0] = s_RenderData.WhiteTexture;

		s_RenderData.QuadVertexPositions[0] = glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
		s_RenderData.QuadVertexPositions[1] = glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);
		s_RenderData.QuadVertexPositions[2] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
		s_RenderData.QuadVertexPositions[3] = glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);

		s_RenderData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraBuffer), 0);
	}

	void Renderer2D::Shutdown()
	{
		//delete s_RenderData;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		PE_PROFILE_FUNCTION();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer2DData::CameraBuffer));

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		PE_PROFILE_FUNCTION();
		glm::mat4 viewProjection = camera.GetViewProjection();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer2DData::CameraBuffer));

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		PE_PROFILE_FUNCTION();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer2DData::CameraBuffer));

		StartNewBatch();
	}

	void Renderer2D::EndScene()
	{
		PE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::Flush()
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.QuadIndexCount) {
			uint32_t dataSize = (uint8_t*)s_RenderData.QuadVertexBufferPtr - (uint8_t*)s_RenderData.QuadVertexBufferBase;
			s_RenderData.QuadVertexBuffer->SetData(s_RenderData.QuadVertexBufferBase, dataSize);

			s_RenderData.QuadShader->Bind();
			for (uint32_t i = 0; i < s_RenderData.TextureSlotIndex; i++) {
				s_RenderData.TextureSlots[i]->Bind(i);
			}
			RenderCommand::DrawIndexed(s_RenderData.QuadVertexArray, s_RenderData.QuadIndexCount);
			s_RenderData.Stats.DrawCalls++;
		}

		if (s_RenderData.CircleIndexCount) {
			uint32_t dataSize = (uint8_t*)s_RenderData.CircleVertexBufferPtr - (uint8_t*)s_RenderData.CircleVertexBufferBase;
			s_RenderData.CircleVertexBuffer->SetData(s_RenderData.CircleVertexBufferBase, dataSize);

			s_RenderData.CircleShader->Bind();
			RenderCommand::DrawIndexed(s_RenderData.CircleVertexArray, s_RenderData.CircleIndexCount);
			s_RenderData.Stats.DrawCalls++;
		}
		
		if (s_RenderData.LineVertexCount) {
			uint32_t dataSize = (uint8_t*)s_RenderData.LineVertexBufferPtr - (uint8_t*)s_RenderData.LineVertexBufferBase;
			s_RenderData.LineVertexBuffer->SetData(s_RenderData.LineVertexBufferBase, dataSize);

			s_RenderData.LineShader->Bind();
			RenderCommand::SetLineWidth(s_RenderData.LineWidth);
			RenderCommand::DrawLines(s_RenderData.LineVertexArray, s_RenderData.LineVertexCount);
			s_RenderData.Stats.DrawCalls++;
		}

		if (s_RenderData.TextIndexCount) {
			uint32_t dataSize = (uint8_t*)s_RenderData.TextVertexBufferPtr - (uint8_t*)s_RenderData.TextVertexBufferBase;
			s_RenderData.TextVertexBuffer->SetData(s_RenderData.TextVertexBufferBase, dataSize);

			s_RenderData.TextShader->Bind();
			s_RenderData.FontAtlasTexture->Bind(0);
			RenderCommand::DrawIndexed(s_RenderData.TextVertexArray, s_RenderData.TextIndexCount);
			s_RenderData.Stats.DrawCalls++;
		}

		StartNewBatch();
	}

	void Renderer2D::StartNewBatch()
	{
		s_RenderData.QuadIndexCount = 0;
		
		s_RenderData.TextureSlotIndex = 1;
		s_RenderData.TextureSlots.fill(Ref<Texture2D>());
		s_RenderData.TextureSlots[0] = s_RenderData.WhiteTexture;

		s_RenderData.QuadVertexBufferPtr = s_RenderData.QuadVertexBufferBase;

		s_RenderData.CircleIndexCount = 0;
		s_RenderData.CircleVertexBufferPtr = s_RenderData.CircleVertexBufferBase;

		s_RenderData.LineVertexCount = 0;
		s_RenderData.LineVertexBufferPtr = s_RenderData.LineVertexBufferBase;

		s_RenderData.TextIndexCount = 0;
		s_RenderData.TextVertexBufferPtr = s_RenderData.TextVertexBufferBase;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.QuadIndexCount >= Renderer2DData::MaxIndices) {
			EndScene();
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

	void Renderer2D::DrawQuad(const glm::mat4& transform, const AssetHandle textureHandle, const glm::vec2& textureScale, const glm::vec4& tintColour, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (textureHandle)
		{
			Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
			DrawQuad(transform, texture, textureScale, tintColour, entityID);
		}
		else {
			DrawQuad(transform, tintColour, entityID);
		}
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subtexture, const glm::vec4& tintColour, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.QuadIndexCount >= Renderer2DData::MaxIndices) {
			EndScene();
		}

		Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(subtexture->GetTextureHandle());
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

		const glm::vec2* textureCoords = subtexture->GetTexCoords();

		for (int i = 0; i < 4; i++) {
			s_RenderData.QuadVertexBufferPtr->Position = transform * s_RenderData.QuadVertexPositions[i];
			s_RenderData.QuadVertexBufferPtr->Colour = tintColour;
			s_RenderData.QuadVertexBufferPtr->TexCoords = textureCoords[i];
			s_RenderData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_RenderData.QuadVertexBufferPtr->TextureScale = glm::vec2(1.0f, 1.0f);
			s_RenderData.QuadVertexBufferPtr->EntityID = entityID;
			s_RenderData.QuadVertexBufferPtr++;
		}

		s_RenderData.QuadIndexCount += 6;

		s_RenderData.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const AssetHandle textureAtlasHandle, const std::string& subtextureName, const glm::vec4& tintColour, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (textureAtlasHandle)
		{
			Ref<TextureAtlas2D> textureAtlas = AssetManager::GetAsset<TextureAtlas2D>(textureAtlasHandle);
			if (subtextureName != "") {
				Ref<SubTexture2D> subtexture = textureAtlas->GetSubTexture(subtextureName);
				if (subtexture) {
					DrawQuad(transform, subtexture, tintColour, entityID);
					return;
				}
			}
			DrawQuad(transform, textureAtlas->GetBaseTexture(), glm::vec2(1.0f, 1.0f), tintColour, entityID);
		}
		else {
			DrawQuad(transform, tintColour, entityID);
		}
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& colour, const float thickness, const float fade, const int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.CircleIndexCount >= Renderer2DData::MaxIndices) {
			EndScene();
		}

		for (int i = 0; i < 4; i++) {
			s_RenderData.CircleVertexBufferPtr->WorldPosition = transform * s_RenderData.QuadVertexPositions[i];
			s_RenderData.CircleVertexBufferPtr->LocalPosition = s_RenderData.QuadVertexPositions[i] * 2.0f;
			s_RenderData.CircleVertexBufferPtr->Colour = colour;
			s_RenderData.CircleVertexBufferPtr->Thickness = thickness;
			s_RenderData.CircleVertexBufferPtr->Fade = fade;
			s_RenderData.CircleVertexBufferPtr->EntityID = entityID;
			s_RenderData.CircleVertexBufferPtr++;
		}

		s_RenderData.CircleIndexCount += 6;

		s_RenderData.Stats.CircleCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& colour, const int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (s_RenderData.LineVertexCount >= Renderer2DData::MaxIndices) {
			EndScene();
		}

		s_RenderData.LineVertexBufferPtr->Position = p0;
		s_RenderData.LineVertexBufferPtr->Colour = colour;
		s_RenderData.LineVertexBufferPtr->EntityID = entityID;

		s_RenderData.LineVertexBufferPtr++;

		s_RenderData.LineVertexBufferPtr->Position = p1;
		s_RenderData.LineVertexBufferPtr->Colour = colour;
		s_RenderData.LineVertexBufferPtr->EntityID = entityID;

		s_RenderData.LineVertexBufferPtr++;

		s_RenderData.LineVertexCount += 2;
		s_RenderData.Stats.LineCount++;
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& colour, const int entityID)
	{
		PE_PROFILE_FUNCTION();

		glm::vec3 lineVertices[4];
		for (int i = 0; i < 4; i++) {
			lineVertices[i] = transform * s_RenderData.QuadVertexPositions[i];
		}

		DrawLine(lineVertices[0], lineVertices[1], colour, entityID);
		DrawLine(lineVertices[1], lineVertices[2], colour, entityID);
		DrawLine(lineVertices[2], lineVertices[3], colour, entityID);
		DrawLine(lineVertices[3], lineVertices[0], colour, entityID);
	}

	void Renderer2D::DrawString(const std::string& string, Ref<Font> font, const glm::mat4& transform, const TextParams& textParams, int entityID)
	{
		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();
		Ref<Texture2D> fontAtlas = font->GetAtlasTexture();

		s_RenderData.FontAtlasTexture = fontAtlas;

		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;

		const float spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();

		for (size_t i = 0; i < string.size(); i++) {
			char character = string[i];
			if (character == '\r') { continue; }
			if (character == '\n') {
				x = 0;
				y -= fsScale * metrics.lineHeight + textParams.LineSpacing;
				continue;
			}

			if (character == ' ') {
				float advance = spaceGlyphAdvance;
				if (i < string.size() - 1) {
					char nextCharacter = string[i + 1];
					double dAdvance;
					fontGeometry.getAdvance(dAdvance, character, nextCharacter);
					advance = (float)dAdvance;
				}

				x += fsScale * advance + textParams.Kerning;
				continue;
			}

			if (character == '\t') {
				x += 4.0f * (fsScale * spaceGlyphAdvance + textParams.Kerning);
				continue;
			}

			auto glyph = fontGeometry.getGlyph(character);
			if (!glyph) {
				glyph = fontGeometry.getGlyph('?');
			}
			if (!glyph) { return; }

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 texCoordMin((float)al, (float)ab);
			glm::vec2 texCoordMax((float)ar, (float)at);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin((float)pl, (float)pb);
			glm::vec2 quadMax((float)pr, (float)pt);

			quadMin *= fsScale, quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / fontAtlas->GetWidth();
			float texelHeight = 1.0f / fontAtlas->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			if (s_RenderData.TextIndexCount >= Renderer2DData::MaxIndices) {
				EndScene();
			}

			// render
			s_RenderData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
			s_RenderData.TextVertexBufferPtr->Colour = textParams.Colour;
			s_RenderData.TextVertexBufferPtr->TexCoords = texCoordMin;
			s_RenderData.TextVertexBufferPtr->EntityID = entityID;
			s_RenderData.TextVertexBufferPtr++;

			s_RenderData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
			s_RenderData.TextVertexBufferPtr->Colour = textParams.Colour;
			s_RenderData.TextVertexBufferPtr->TexCoords = { texCoordMin.x, texCoordMax.y };
			s_RenderData.TextVertexBufferPtr->EntityID = entityID;
			s_RenderData.TextVertexBufferPtr++;

			s_RenderData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
			s_RenderData.TextVertexBufferPtr->Colour = textParams.Colour;
			s_RenderData.TextVertexBufferPtr->TexCoords = texCoordMax;
			s_RenderData.TextVertexBufferPtr->EntityID = entityID;
			s_RenderData.TextVertexBufferPtr++;

			s_RenderData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
			s_RenderData.TextVertexBufferPtr->Colour = textParams.Colour;
			s_RenderData.TextVertexBufferPtr->TexCoords = { texCoordMax.x, texCoordMin.y };
			s_RenderData.TextVertexBufferPtr->EntityID = entityID;
			s_RenderData.TextVertexBufferPtr++;

			s_RenderData.TextIndexCount += 6;
			s_RenderData.Stats.QuadCount++;

			if (i < string.size() - 1) {
				double advance = glyph->getAdvance();
				char nextCharacter = string[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter);
				
				x += fsScale * advance + textParams.Kerning;
			}
		}
	}

	void Renderer2D::DrawString(const std::string& string, AssetHandle fontHandle, const glm::mat4& transform, const TextParams& textParams, int entityID)
	{
		PE_PROFILE_FUNCTION();

		if (fontHandle)
		{
			Ref<Font> font = AssetManager::GetAsset<Font>(fontHandle);
			DrawString(string, font, transform, textParams, entityID);
		}
		else {
			DrawString(string, AssetManager::GetAsset<Font>(Font::s_DefaultFont), transform, textParams, entityID);
		}
	}

	float Renderer2D::GetLineWidth()
	{
		return s_RenderData.LineWidth;
	}

	void Renderer2D::SetLineWidth(const float thickness)
	{
		s_RenderData.LineWidth = thickness;
	}

	void Renderer2D::ResetStats()
	{
		s_RenderData.Stats.DrawCalls = 0;
		s_RenderData.Stats.QuadCount = 0;
		s_RenderData.Stats.CircleCount = 0;
		s_RenderData.Stats.LineCount = 0;
	}

	const Renderer2D::Statistics& Renderer2D::GetStats()
	{
		return s_RenderData.Stats;
	}
}