#pragma once
#include <vector>
#include "MeshData.h"
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <iostream>
#include "ResourceManager.h"
namespace Engine {
	class ASSIMPModelLoader
	{
	public:
		static const std::vector<MeshData*> LoadMeshData(const std::string& filepath, unsigned assimpPostProcess, bool persistentResources = false);
		static std::vector<MeshData*> ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, bool persistentResources = false);
		static MeshData* ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, bool persistentResources = false);
		//static void ProcessBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
		//static bool ProcessEmptyBones(aiNode* node);
		//static std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name, const aiScene* scene);

		static std::vector<Texture*> LoadMaterialTextures(const std::string& filepath, const aiMaterial* mat, const aiTextureType type, const TextureTypes name, const aiScene* scene);

		static PBRMaterial* LoadPBRMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath);
		static Material* LoadMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath);

		static bool loadMaterialsAsPBR;
	};
}