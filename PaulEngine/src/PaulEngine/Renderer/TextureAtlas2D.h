#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "SubTexture2D.h"

namespace PaulEngine
{
	class TextureAtlas2D : public Asset
	{
	public:
		TextureAtlas2D(AssetHandle baseTexture);

		AssetHandle GetBaseTexture() const { return m_BaseTextureHandle; }

		void AddSubTexture(const std::string& name, SubTexture2D subtexture);
		Ref<SubTexture2D> GetSubTexture(const std::string& name);

		const std::vector<std::string>& GetSubTextureNames() const { return m_SubTextureNames; }

		virtual AssetType GetType() const override { return AssetType::TextureAtlas2D; }

	private:
		friend class TextureImporter;
		AssetHandle m_BaseTextureHandle;
		std::unordered_map<std::string, SubTexture2D> m_SubTextureMap;
		std::vector<std::string> m_SubTextureNames;
	};
}