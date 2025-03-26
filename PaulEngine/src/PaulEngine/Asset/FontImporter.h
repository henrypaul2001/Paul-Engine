#pragma once
#include "Asset.h"
#include "PaulEngine/Renderer/Font.h"

namespace PaulEngine
{
	class FontImporter
	{
	public:
		static Ref<Font> ImportFont(AssetHandle handle, const AssetMetadata& metadata);

		static Ref<Font> LoadFont(const std::filesystem::path& filepath, const bool persistentTextureAtlas);
	};
}