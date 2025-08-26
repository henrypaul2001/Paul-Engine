#pragma once
#include "Asset.h"
#include "PaulEngine/Renderer/Asset/Mesh.h"

namespace PaulEngine
{
	struct ParsedMesh
	{
		std::vector<MeshVertex> Vertices;
		std::vector<uint32_t> Indices;
	};
	struct ParsedModelLoadResult
	{
		std::vector<ParsedMesh> ParsedMeshes;
	};

	class MeshImporter
	{
	public:
		static Ref<Mesh> ImportMesh(AssetHandle handle, const AssetMetadata& metadata);

		// Creats a .pmesh file for every submesh found inside of the base model file
		static Ref<Model> ImportModel(AssetHandle handle, const AssetMetadata& metadata);

		static Ref<Mesh> LoadMesh(const std::filesystem::path& filepath);
		static Ref<Model> LoadModel(const std::filesystem::path& filepath);

		static void CreatePrefabFromImportedModel(AssetHandle modelHandle);

		// Experimental work for batched meshes
		static Ref<ParsedModelLoadResult> ParseModelFileRaw(const std::filesystem::path& filepath);

	private:
		static std::filesystem::path CreateMeshFile(const std::filesystem::path& baseModelFilepath, const Ref<Mesh>& loadedMesh, uint32_t meshIndex, const AssetHandle modelHandle, const bool persistent);
		static AssetHandle CreateAndImportMeshFile(const std::filesystem::path& baseModelFilepath, const Ref<Mesh>& loadedMesh, uint32_t meshIndex, const AssetHandle modelHandle, const bool persistent);
	};
}