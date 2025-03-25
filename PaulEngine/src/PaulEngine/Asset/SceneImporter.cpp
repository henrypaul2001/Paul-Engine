#include "pepch.h"
#include "SceneImporter.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Scene/SceneSerializer.h"

#include <stb_image.h>

namespace PaulEngine
{
	Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		return LoadScene(Project::GetAssetDirectory() / metadata.FilePath);
	}

	Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();

		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer = SceneSerializer(scene);
		serializer.DeserializeYAML(path);
		return scene;
	}

	void SceneImporter::SaveScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer = SceneSerializer(scene);
		serializer.SerializeYAML(Project::GetAssetDirectory() / path);
	}
}