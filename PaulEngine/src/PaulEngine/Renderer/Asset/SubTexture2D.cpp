#include "pepch.h"
#include "SubTexture2D.h"

#include "PaulEngine/Asset/AssetManager.h"

namespace PaulEngine {
	SubTexture2D::SubTexture2D(AssetHandle textureHandle, const glm::vec2& min, const glm::vec2& max) : m_TextureHandle(textureHandle)
	{
		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

	SubTexture2D SubTexture2D::CreateFromCoords(AssetHandle textureHandle, const glm::vec2& xy, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		PE_PROFILE_FUNCTION();

		Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
		PE_CORE_ASSERT(texture, "Invalid texture handle");

		const float sheetWidth = (float)texture->GetWidth();
		const float sheetHeight = (float)texture->GetHeight();

		const float x = xy.x;
		const float y = xy.y;

		const glm::vec2 min = glm::vec2((x * cellSize.x) / sheetWidth, (y * cellSize.y) / sheetHeight);
		const glm::vec2 max = glm::vec2(((x + spriteSize.x) * cellSize.x) / sheetWidth, ((y + spriteSize.y) * cellSize.y) / sheetHeight);

		return SubTexture2D(textureHandle, min, max);
	}
}