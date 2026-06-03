#include "pepch.h"
#include "SceneImporter.h"

#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Scene/SceneSerializer.h"
#include "PaulEngine/Scene/Components.h"

#include <stb_image.h>

namespace PaulEngine
{
	Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Scene> scene = LoadScene(Project::GetAssetDirectory() / metadata.FilePath);
		scene->Handle = handle;
		return scene;
	}

	Ref<Prefab> SceneImporter::ImportPrefab(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Prefab> prefab = LoadPrefab(Project::GetAssetDirectory() / metadata.FilePath);
		prefab->Handle = handle;
		return prefab;
	}

	Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();

		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer = SceneSerializer(scene);
		serializer.DeserializeYAML(path);

		// Load prefabs
		auto view = scene->View<ComponentPrefabSource>();
		for (auto entityID : view) {
			ComponentPrefabSource& prefab = view.get<ComponentPrefabSource>(entityID);
			Ref<Prefab> prefabAsset = AssetManager::GetAsset<Prefab>(prefab.PrefabHandle);
			if (prefabAsset) {
				prefabAsset->RefreshInstance(Entity(entityID, scene.get()));
			}
		}

		return scene;
	}

	Ref<Prefab> SceneImporter::LoadPrefab(const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();
		Ref<Prefab> prefab = CreateRef<Prefab>();
		prefab->m_PrefabScene = LoadScene(path);
		return prefab;
	}

	void SceneImporter::SaveScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer = SceneSerializer(scene);
		serializer.SerializeYAML(Project::GetAssetDirectory() / path, true);
	}

	void SceneImporter::SavePrefab(Prefab prefab, const std::filesystem::path& path)
	{
		SceneSerializer serializer = SceneSerializer(prefab.GetPrefabScene());
		serializer.SerializeYAML(Project::GetAssetDirectory() / path, false);
	}
}