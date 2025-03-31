#pragma once
#include "Texture.h"
#include <glm/glm.hpp>

namespace PaulEngine {
	class SubTexture2D
	{
	public:
		SubTexture2D(AssetHandle textureHandle, const glm::vec2& min, const glm::vec2& max);

		AssetHandle GetTextureHandle() const { return m_TextureHandle; }
		const glm::vec2* GetTexCoords() const { return m_TexCoords; }

		static SubTexture2D CreateFromCoords(AssetHandle textureHandle, const glm::vec2& xy, const glm::vec2& cellSize, const glm::vec2& spriteSize = glm::vec2(1.0f));
	private:
		AssetHandle m_TextureHandle = 0;
		glm::vec2 m_TexCoords[4];
	};
}