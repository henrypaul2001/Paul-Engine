#pragma once
#include "PaulEngine/Renderer/RenderPipeline.h"
#include "PaulEngine/Renderer/OrthographicCamera.h"
#include "PaulEngine/Renderer/Texture.h"
#include "PaulEngine/Renderer/SubTexture2D.h"
#include "PaulEngine/Renderer/Camera.h"
#include "PaulEngine/Renderer/EditorCamera.h"
#include "PaulEngine/Renderer/Font.h"

namespace PaulEngine {
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera, FaceCulling cullState = FaceCulling::BACK, DepthState depthState = { DepthFunc::LESS, true, true });
		static void BeginScene(const EditorCamera& camera, FaceCulling cullState = FaceCulling::BACK, DepthState depthState = {DepthFunc::LESS, true, true });
		static void BeginScene(const Camera& camera, const glm::mat4& worldTransform, FaceCulling cullState = FaceCulling::BACK, DepthState depthState = { DepthFunc::LESS, true, true });
		static void BeginScene(const glm::mat4& projection, const glm::mat4& worldTransform, float gamma = 2.2f, FaceCulling cullState = FaceCulling::BACK, DepthState depthState = { DepthFunc::LESS, true, true });
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& colour, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture>& texture, const glm::vec2& textureScale = glm::vec2(1.0f), const glm::vec4& tintColour = glm::vec4(1.0f), int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const AssetHandle textureHandle, const glm::vec2& textureScale = glm::vec2(1.0f), const glm::vec4& tintColour = glm::vec4(1.0f), int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subtexture, const glm::vec4& tintColour = glm::vec4(1.0f), int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const AssetHandle textureAtlasHandle, const std::string& subtextureName, const glm::vec4& tintColour = glm::vec4(1.0f), int entityID = -1);

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& colour, const float thickness, const float fade, const int entityID = -1);
		
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& colour, const int entityID = -1);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& colour, const int entityID = -1);

		struct TextParams {
			glm::vec4 Colour = glm::vec4(1.0f);
			float Kerning = 0.0f;
			float LineSpacing = 0.0f;
		};
		static void DrawString(const std::string& string, Ref<Font> font, const glm::mat4& transform, const TextParams& textParams, int entityID = -1);
		static void DrawString(const std::string& string, AssetHandle fontHandle , const glm::mat4& transform, const TextParams& textParams, int entityID = -1);

		static float GetLineWidth();
		static void SetLineWidth(const float thickness);

		struct Statistics {
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t CircleCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};
		static void ResetStats();
		static const Statistics& GetStats();

		static void ImportShaders();

		static Ref<Texture2D> GetWhiteTexture();

	private:
		static void StartNewBatch();
	};
}