#include "pepch.h"
#include "TextureAtlas2D.h"

namespace PaulEngine
{
	TextureAtlas2D::TextureAtlas2D(AssetHandle baseTexture) : m_BaseTextureHandle(baseTexture) {}

	void TextureAtlas2D::SetSubTexture(const std::string& name, SubTexture2D subtexture)
	{
		m_SubTextureMap[name] = subtexture;
	}

	SubTexture2D TextureAtlas2D::GetSubTexture(const std::string& name)
	{
		if (m_SubTextureMap.find(name) != m_SubTextureMap.end()) {
			return m_SubTextureMap[name];
		}
		else {
			PE_CORE_WARN("Unknown subtexture: '{0}'", name.c_str());
		}
	}
}