#pragma once
#include "AssetManagerBase.h"
#include "PaulEngine/Project/Project.h"

namespace PaulEngine
{
	class AssetManager
	{
	public:
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);
		}

		template <typename T, typename... Args>
		static Ref<T> CreateAsset(bool persistent, Args&&... args)
		{
			return Project::GetActive()->GetAssetManager()->CreateAsset<T>(persistent, std::forward<Args>(args)...);
		}

		static bool IsAssetHandleValid(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetRegistered(handle);
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetLoaded(handle);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->GetAssetType(handle);
		}

		static const AssetMetadata& GetMetadata(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->GetMetadata(handle);
		}

		static bool IsAssetProcedural(AssetHandle handle)
		{
			return Project::GetActive()->GetAssetManager()->IsAssetProcedural(handle);
		}

		static void UnloadAsset(AssetHandle& handle)
		{
			Project::GetActive()->GetAssetManager()->UnloadAsset(handle);
		}
	};
}