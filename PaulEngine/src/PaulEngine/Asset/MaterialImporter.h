#pragma once
#include "Asset.h"
#include "PaulEngine/Renderer/Asset/Material.h"

struct aiScene;
namespace PaulEngine
{
	class MaterialImporter
	{
	public:
		static Ref<Material> ImportMaterial(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Material> LoadMaterial(const std::filesystem::path& filepath);
		static void SaveMaterial(const Ref<Material> material, const std::filesystem::path& filepath);
		static void ImportMaterialsFromModelFile(const std::filesystem::path& filepath, bool persistent);
		static void ImportMaterialsFromModelFile(const std::filesystem::path& filepath, bool persistent, std::vector<AssetHandle>& out_results);
		static void ImportMaterialsFromModelFile(const aiScene* scene, const std::filesystem::path& filepath, bool persistent, std::vector<AssetHandle>& out_results);
	};
}