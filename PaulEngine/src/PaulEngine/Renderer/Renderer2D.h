#pragma once
#include "PaulEngine/Renderer/OrthographicCamera.h"
#include "PaulEngine/Renderer/Texture.h"

namespace PaulEngine {
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& colour);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& colour);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture>& texture, const glm::vec2& textureScale = glm::vec2(1.0f), const glm::vec4& tintColour = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture, const glm::vec2& textureScale = glm::vec2(1.0f), const glm::vec4& tintColour = glm::vec4(1.0f));
	};
}