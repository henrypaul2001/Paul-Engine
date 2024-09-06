#pragma once
#include <vector>
#include "MeshData.h"
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <iostream>
#include "ResourceManager.h"
namespace Engine {
	struct ProcessMeshResult {
		MeshData* meshData;
		std::vector<AbstractMaterial*> meshMaterials;
	};
	
	class ASSIMPModelLoader
	{
	public:
		static const std::vector<ProcessMeshResult> LoadMeshData(const std::string& filepath, unsigned assimpPostProcess, bool persistentResources = false);
		static std::vector<ProcessMeshResult> ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, bool persistentResources = false);
		static ProcessMeshResult ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, bool persistentResources = false);
		//static void ProcessBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
		//static bool ProcessEmptyBones(aiNode* node);

		static std::vector<Texture*> LoadMaterialTextures(const std::string& filepath, const aiMaterial* mat, const aiTextureType type, const TextureTypes name, const aiScene* scene);

		static PBRMaterial* LoadPBRMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath);
		static Material* LoadMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath);

		static bool loadMaterialsAsPBR;
	};
}