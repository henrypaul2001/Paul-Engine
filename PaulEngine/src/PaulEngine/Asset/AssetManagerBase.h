#pragma once
#include "Asset.h"
#include <unordered_map>
#include <concepts>

namespace PaulEngine
{
	template <typename T, typename... Args>
	concept HasCreateFunc = requires(Args&&... args)
	{
		{ T::Create(std::forward<Args>(args)...) } -> std::convertible_to<Ref<Asset>>;
	};

	using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

	// Interface API for different asset managers such as runtime assets vs editor assets
	class AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
		virtual const AssetMetadata& GetMetadata(AssetHandle handle) const = 0;
		virtual AssetType GetAssetType(AssetHandle handle) const = 0;
		virtual bool IsAssetRegistered(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
		virtual bool IsAssetProcedural(AssetHandle handle) const = 0;
		virtual void UnloadAsset(AssetHandle& handle) = 0;
		virtual void ReleaseTempAssets() = 0;
		virtual void Clear() = 0;
		virtual void RegisterAsset(AssetHandle handle, AssetMetadata metadata) = 0;

		template <typename T, typename... Args>
		Ref<T> CreateAsset(bool persistent, Args&&... args)
		{
			Ref<Asset> asset;

			// If T implements the static Create() function found in classes like Texture2D, call that. Otherwise, call the constructor
			if constexpr (HasCreateFunc<T, Args&&...>)
			{
				asset = T::Create(std::forward<Args>(args)...);
			}
			else
			{
				asset = CreateRef<T>(std::forward<Args>(args)...);
			}

			AddToLoadedAssets(asset, persistent);
			AssetMetadata metadata;
			metadata.Type = asset->GetType();
			metadata.Persistent = persistent;
			RegisterAsset(asset->Handle, metadata);
			return std::static_pointer_cast<T>(asset);
		}

	protected:
		virtual void AddToLoadedAssets(Ref<Asset> asset, bool persistent = false) = 0;
	};
}