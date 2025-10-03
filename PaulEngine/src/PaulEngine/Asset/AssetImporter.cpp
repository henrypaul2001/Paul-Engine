#include "pepch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "SceneImporter.h"
#include "FontImporter.h"
#include "ShaderImporter.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"

#include <unordered_map>

#include "PaulEngine/Asset/BinarySerializer.h"

namespace PaulEngine
{

	static std::filesystem::path GetAssetFileCacheDirectory()
	{
		return "assets/cache/asset_file";
	}
	static std::filesystem::path GetAssetFileCachePath(AssetHandle handle)
	{
		return GetAssetFileCacheDirectory() / std::filesystem::path(std::to_string(handle));
	}
	static bool IsAssetFileCached(AssetHandle handle)
	{
		return std::filesystem::exists(GetAssetFileCachePath(handle));
	}

	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;

	static std::unordered_map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Texture2D, TextureImporter::ImportTexture2D },
		{ AssetType::Scene, SceneImporter::ImportScene },
		{ AssetType::Font, FontImporter::ImportFont },
		{ AssetType::TextureAtlas2D, TextureImporter::ImportTextureAtlas2D },
		{ AssetType::Shader, ShaderImporter::ImportShader },
		{ AssetType::Material, MaterialImporter::ImportMaterial },
		{ AssetType::Texture2DArray, TextureImporter::ImportTexture2DArray },
		{ AssetType::Mesh, MeshImporter::ImportMesh },
		{ AssetType::Model, MeshImporter::ImportModel },
		{ AssetType::Prefab, SceneImporter::ImportPrefab },
		{ AssetType::EnvironmentMap, TextureImporter::ImportEnvironmentMap },
		{ AssetType::TextureCubemap, TextureImporter::ImportTextureCubemap }
	};

	Ref<Asset> DeserializeAssetFromType(AssetType type, std::istream& stream)
	{
		switch (type)
		{
			case AssetType::EnvironmentMap:
			{
				EnvironmentMap envMap = EnvironmentMap();
				bool success = BinarySerializer::DeserializeAssetBinaryData(envMap, stream);
				if (!success) { return nullptr; }
				return CreateRef<EnvironmentMap>(envMap);
			}
		}
		PE_CORE_ERROR("Undefined asset type to binary deserializer translation");
		return nullptr;
	}

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		// Load binary asset file if it exists
		std::filesystem::path cachedPath = GetAssetFileCachePath(handle);
		if (std::filesystem::exists(cachedPath))
		{
			std::ifstream filestream = std::ifstream(cachedPath, std::ios_base::binary);
			size_t dataSize = 0;
			AssetType assetType = AssetType::None;

			bool headerSuccess = BinarySerializer::DeserializePAssetHeader(filestream, dataSize, assetType);
			if (!headerSuccess)
			{
				PE_CORE_ERROR("Error deserializing cached asset file header");
				filestream.close();
				return ImportAssetFromSource(handle, metadata);
			}

			Ref<Asset> assetInstance = DeserializeAssetFromType(assetType, filestream);
			if (!assetInstance)
			{
				PE_CORE_ERROR("Error deserializing cached asset file data");
				filestream.close();
				return ImportAssetFromSource(handle, metadata);
			}

			assetInstance->Handle = handle;
			return assetInstance;
		}
		else { return ImportAssetFromSource(handle, metadata); }
	}

	Ref<Asset> AssetImporter::ImportAssetFromSource(AssetHandle handle, const AssetMetadata& metadata)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end()) {
			PE_CORE_ERROR("No importer available for asset type: {}", (uint16_t)metadata.Type);
			return nullptr;
		}

		return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
	}
}