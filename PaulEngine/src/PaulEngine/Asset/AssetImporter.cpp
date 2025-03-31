#include "pepch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "SceneImporter.h"
#include "FontImporter.h"

#include <unordered_map>

namespace PaulEngine
{
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;

	static std::unordered_map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Texture2D, TextureImporter::ImportTexture2D },
		{ AssetType::Scene, SceneImporter::ImportScene },
		{ AssetType::Font, FontImporter::ImportFont },
		{ AssetType::TextureAtlas2D, TextureImporter::ImportTextureAtlas2D }
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end()) {
			PE_CORE_ERROR("No importer available for asset type: {}", (uint16_t)metadata.Type);
			return nullptr;
		}

		return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
	}
}