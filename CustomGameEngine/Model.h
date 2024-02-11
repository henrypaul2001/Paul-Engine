#pragma once
#include "Mesh.h"
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace Engine {
	class Model
	{
	public:
		Model(const char* filepath);
		Model(const char* filepath, bool pbr);
		~Model();
		void Draw(Shader& shader);

		bool PBR() { return pbr; }

		std::vector<Mesh> meshes;
		std::vector<Texture> textures_loaded;
	private:
		std::string directory;
		bool pbr;

		void LoadModel(std::string filepath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name);
		unsigned int TextureFromFile(const char* path, const std::string& directory);
	};
}