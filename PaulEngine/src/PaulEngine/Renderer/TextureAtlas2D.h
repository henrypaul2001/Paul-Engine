#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "SubTexture2D.h"

namespace PaulEngine
{
	class TextureAtlas2D : public Asset
	{
	public:
		TextureAtlas2D(AssetHandle baseTexture);

		void SetSubTexture(const std::string& name, SubTexture2D subtexture);
		SubTexture2D GetSubTexture(const std::string& name);

		virtual AssetType GetType() const override { return AssetType::TextureAtlas2D; }

	private:
		friend class TextureImporter;
		AssetHandle m_BaseTextureHandle;
		std::unordered_map<std::string, SubTexture2D> m_SubTextureMap;
	};
}