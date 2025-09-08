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
			Ref<Project> project = Project::GetActive();
			if (project)
			{
				Ref<Asset> asset = project->GetAssetManager()->GetAsset(handle);
				return std::static_pointer_cast<T>(asset);
			}
			else
			{
				return nullptr;
			}
		}

		template <typename T, typename... Args>
		static Ref<T> CreateAsset(bool persistent, Args&&... args)
		{
			Ref<Project> project = Project::GetActive();
			if (project) { return project->GetAssetManager()->CreateAsset<T>(persistent, std::forward<Args>(args)...); }
			return nullptr;
		}

		static bool IsAssetHandleValid(AssetHandle handle)
		{
			Ref<Project> project = Project::GetActive();
			if (project) { return project->GetAssetManager()->IsAssetRegistered(handle); }
			return false;
		}

		static bool IsAssetLoaded(AssetHandle handle)
		{
			Ref<Project> project = Project::GetActive();
			if (project) { return project->GetAssetManager()->IsAssetLoaded(handle); }
			return false;
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			Ref<Project> project = Project::GetActive();
			if (project) { return project->GetAssetManager()->GetAssetType(handle); }
			return AssetType::None;
		}

		static const AssetMetadata& GetMetadata(AssetHandle handle)
		{
			Ref<Project> project = Project::GetActive();
			PE_CORE_ASSERT(project, "No active project");
			return project->GetAssetManager()->GetMetadata(handle);
		}

		static bool IsAssetProcedural(AssetHandle handle)
		{
			Ref<Project> project = Project::GetActive();
			if (project) { return project->GetAssetManager()->IsAssetProcedural(handle); }
			return false;
		}

		static void UnloadAsset(AssetHandle& handle)
		{
			Ref<Project> project = Project::GetActive();
			if (project) { project->GetAssetManager()->UnloadAsset(handle); }
		}

		static void ReleaseTempAssets()
		{
			Ref<Project> project = Project::GetActive();
			if (project) { project->GetAssetManager()->ReleaseTempAssets(); }
		}

		static void Clear()
		{
			Ref<Project> project = Project::GetActive();
			if (project) { project->GetAssetManager()->Clear(); }
		}
	};
}