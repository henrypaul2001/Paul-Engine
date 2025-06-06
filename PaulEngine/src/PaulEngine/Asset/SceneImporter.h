#pragma once
#include "Asset.h"
#include "PaulEngine/Scene/Scene.h"
#include "PaulEngine/Scene/Prefab.h"

namespace PaulEngine
{
	class SceneImporter
	{
	public:
		static Ref<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Prefab> ImportPrefab(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Scene> LoadScene(const std::filesystem::path& path);
		static Ref<Prefab> LoadPrefab(const std::filesystem::path& path);
		
		static void SaveScene(Ref<Scene> scene, const std::filesystem::path& path);
		static void SavePrefab(Prefab prefab, const std::filesystem::path& path);
	};
}