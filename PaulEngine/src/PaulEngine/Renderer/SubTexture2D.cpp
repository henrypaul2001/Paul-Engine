#include "pepch.h"
#include "SubTexture2D.h"

namespace PaulEngine {
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max) : m_Texture(texture)
	{
		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& xy, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		PE_PROFILE_FUNCTION();

		const float sheetWidth = (float)texture->GetWidth();
		const float sheetHeight = (float)texture->GetHeight();

		const float x = xy.x;
		const float y = xy.y;

		const glm::vec2 min = glm::vec2((x * cellSize.x) / sheetWidth, (y * cellSize.y) / sheetHeight);
		const glm::vec2 max = glm::vec2(((x + spriteSize.x) * cellSize.x) / sheetWidth, ((y + spriteSize.y) * cellSize.y) / sheetHeight);

		return CreateRef<SubTexture2D>(texture, min, max);
	}
}