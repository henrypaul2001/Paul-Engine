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
	
	struct LoadMeshDataResult {
		std::vector<ProcessMeshResult> meshResults;
		AnimationSkeleton* skeleton;
		bool hasBones;
		bool skeletonAlreadyLoaded;
	};

	class ASSIMPModelLoader
	{
	public:
		static const LoadMeshDataResult LoadMeshData(const std::string& filepath, unsigned assimpPostProcess, bool persistentResources = false);
		static void ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, LoadMeshDataResult& out_result, bool persistentResources = false);
		static void ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, LoadMeshDataResult& out_result, bool persistentResources = false);

		static void ProcessBones(std::vector<Vertex>& vertices, const aiMesh* mesh, const aiScene* scene, LoadMeshDataResult& out_result);
		static bool ProcessEmptyBones(aiNode* node, LoadMeshDataResult& out_result);

		static std::vector<Texture*> LoadMaterialTextures(const std::string& filepath, const aiMaterial* mat, const aiTextureType type, const TextureTypes name, const aiScene* scene);

		static PBRMaterial* LoadPBRMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath);
		static Material* LoadMaterialFromaiMat(const aiMaterial* material, const aiScene* scene, const std::string& filepath);

		// Helper function to convert ASSIMP aiMatrix4x4 to glm::mat4
		static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
		{
			glm::mat4 to;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}

		static bool loadMaterialsAsPBR;
	};
}