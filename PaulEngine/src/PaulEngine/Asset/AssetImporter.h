#pragma once
#include "Asset.h"

namespace PaulEngine
{
	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Asset> ImportAssetFromSource(AssetHandle handle, const AssetMetadata& metadata);

		// Calls the appropriate binary serialize function depending on asset type
		static bool SerializeAssetFromType(const Ref<Asset>& asset);

		static std::filesystem::path GetAssetFileCacheDirectory()
		{
			return "assets/cache/asset_file";
		}
		static void ValidateAssetFileCacheDirectory()
		{
			std::filesystem::path cacheDirectory = GetAssetFileCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
			{
				PE_CORE_DEBUG("Creating directory '{0}'", cacheDirectory.string().c_str());
				std::filesystem::create_directories(cacheDirectory);
			}
		}
		static std::filesystem::path GetAssetFileCachePath(AssetHandle handle)
		{
			return GetAssetFileCacheDirectory() / std::filesystem::path(std::to_string(handle) + ".passet");
		}
		static bool IsAssetFileCached(AssetHandle handle)
		{
			return std::filesystem::exists(GetAssetFileCachePath(handle));
		}
	};
}