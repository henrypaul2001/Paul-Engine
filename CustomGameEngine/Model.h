#pragma once
#include "Mesh.h"

#include <vector>
#include <string>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace Engine {
	class Model
	{
	public:
		Model();
		~Model();
		void Draw();

		std::vector<Mesh> meshes;
		std::vector<Texture> textures;
	private:
		std::string directory;

		void LoadModel(const char* filepath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* typeName);
		unsigned int TextureFromFile(const char* path, const std::string& directory);
	};
}