#include "pepch.h"
#include "TextureAtlas2D.h"

namespace PaulEngine
{
	TextureAtlas2D::TextureAtlas2D(AssetHandle baseTexture) : m_BaseTextureHandle(baseTexture) {}

	void TextureAtlas2D::AddSubTexture(const std::string& name, SubTexture2D subtexture)
	{
		PE_CORE_ASSERT(name != "", "Name cannot be empty");
		PE_CORE_ASSERT(m_SubTextureMap.find(name) == m_SubTextureMap.end(), "SubTexture name already exists in texture atlas");

		m_SubTextureMap[name] = subtexture;
		m_SubTextureNames.push_back(name);
	}

	Ref<SubTexture2D> TextureAtlas2D::GetSubTexture(const std::string& name)
	{
		if (m_SubTextureMap.find(name) != m_SubTextureMap.end()) {
			return CreateRef<SubTexture2D>(m_SubTextureMap[name]);
		}
		else {
			PE_CORE_ERROR("Unknown subtexture: '{0}'", name.c_str());
			return nullptr;
		}
	}
}