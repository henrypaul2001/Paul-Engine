#pragma once
#include "Asset.h"
#include "PaulEngine/Renderer/Asset/Mesh.h"

namespace PaulEngine
{
	class MeshImporter
	{
	public:
		static Ref<Mesh> ImportMesh(AssetHandle handle, const AssetMetadata& metadata);

		// Creats a .pmesh file for every submesh found inside of the base model file
		static Ref<Model> ImportModel(AssetHandle handle, const AssetMetadata& metadata);

		static Ref<Mesh> LoadMesh(const std::filesystem::path& filepath);
		static Ref<Model> LoadModel(const std::filesystem::path& filepath);

	private:
		void CreateMeshFile();
	};
}

// .pmesh file
// Source: (Model file asset handle)
// Index: (submesh index)

// On load, read the original model file and retrieve the indexed submesh
// Performance wise this isn't great. In the case of a large number of submeshes, all submeshes will be parsed by assimp every time one of the meshes is loaded.
// Instead, maybe a better solution would be to load all meshes once in the event that any one of the meshes are requested

// Import mesh:
// Ref<Model> baseModel = AssetManager::GetAsset<Model>(.pmesh base model AssetHandle);
// return baseModel->GetSubmesh(.pmesh submesh index);

// Import model:
// Ref<Model> loadedModel = LoadModelFile();
// Create .pmesh files
// In the future, this would also create a prefab file (when they are supported) that has the correct parent/child relationships for the meshes