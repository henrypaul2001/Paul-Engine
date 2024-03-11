#pragma once
#include "Mesh.h"
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace Engine {
	enum PremadeModel {
		MODEL_SPHERE,
		MODEL_CUBE,
		MODEL_PLANE
	};

	class Model
	{
	public:
		Model(PremadeModel modelType);
		Model(const char* filepath);
		Model(const char* filepath, bool pbr);
		~Model();

		void Draw(Shader& shader);
		void DrawTransparentMeshes(Shader& shader);

		bool PBR() { return pbr; }

		void ApplyMaterialToAllMesh(Material* material);
		void ApplyMaterialToMeshAtIndex(Material* material, int index);

		bool ContainsTransparentMeshes() { return containsTransparentMeshes; }

		std::vector<Mesh*> meshes;
		//std::vector<Texture> textures_loaded;
	private:
		std::string directory;
		bool pbr;

		bool containsTransparentMeshes;

		void LoadModel(std::string filepath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name);
	};
}