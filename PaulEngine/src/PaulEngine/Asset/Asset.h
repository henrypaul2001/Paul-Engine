#pragma once
#include "PaulEngine/Core/UUID.h"

#include <filesystem>

namespace PaulEngine
{
	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Texture2D,
		Font,
		TextureAtlas2D,
		Material,
		Shader,
		Texture2DArray,
		TextureCubemap,
		TextureCubemapArray,
		Mesh,
		Model,
		Prefab,
		EnvironmentMap
	};

	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
		bool Persistent = false;

		operator bool() const { return Type != AssetType::None; }
	};

	std::string AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(const std::string& assetType);

	class Asset
	{
	public:
		AssetHandle Handle;
		virtual ~Asset() {}

		virtual AssetType GetType() const = 0;
		virtual void OnImport() {}

		static constexpr bool IsTextureType(AssetType type) { return (type == AssetType::Texture2D || type == AssetType::Texture2DArray || type == AssetType::TextureCubemap || type == AssetType::TextureCubemapArray); }
	};
}