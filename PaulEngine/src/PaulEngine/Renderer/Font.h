#pragma once
#include <filesystem>
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Renderer/Texture.h"

namespace PaulEngine
{
	struct MSDFData;

	class Font : public Asset
	{
	public:
		Font();
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data; }

		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }
		static Ref<Font> GetDefault();

		virtual AssetType GetType() const override { return AssetType::Font; }
	private:
		friend class FontImporter;
		MSDFData* m_Data;
		Ref<Texture2D> m_AtlasTexture; // TODO (future optimisation): cache texture atlas on disk as a registered asset to avoid regenerating every time a font is loaded
	};
}