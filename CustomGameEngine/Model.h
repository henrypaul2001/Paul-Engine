#pragma once
#include "Mesh.h"
#include "stb_image.h"
#include <map>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace Engine {
	enum PremadeModel {
		MODEL_SPHERE,
		MODEL_CUBE,
		MODEL_PLANE
	};

	struct AnimationBone {
		int boneID = -1;
		std::string name;
		glm::mat4 offsetMatrix = glm::mat4(1.0f);
		glm::mat4 finalTransform = glm::mat4(1.0f);
		glm::mat4 nodeTransform = glm::mat4(1.0f);

		std::vector<std::string> childNodeNames;
	};

	struct AnimationSkeleton {
		std::map<std::string, AnimationBone> bones;
		AnimationBone* rootBone;
		glm::mat4 originTransform;
	};

	class Model
	{
	public:
		Model(PremadeModel modelType, bool pbr = false);
		Model(const char* filepath);
		Model(const char* filepath, bool pbr);
		~Model();

		void Draw(Shader& shader, int instanceNum);
		void DrawTransparentMeshes(Shader& shader, int instanceNum);

		bool PBR() { return pbr; }
		void PBR(bool PBR) { pbr = PBR; }

		void ApplyMaterialToAllMesh(Material* material);
		void ApplyMaterialToMeshAtIndex(Material* material, int index);

		void ApplyMaterialToAllMesh(PBRMaterial* pbrMaterial);
		void ApplyMaterialToMeshAtIndex(PBRMaterial* pbrMaterial, int index);

		bool ContainsTransparentMeshes() { return containsTransparentMeshes; }

		AnimationSkeleton& GetAnimationSkeleton() { return skeleton; }

		std::vector<Mesh*> meshes;
		//std::vector<Texture> textures_loaded;
	private:
		std::string directory;
		bool pbr;

		bool containsTransparentMeshes;

		AnimationSkeleton skeleton;

		void LoadModel(std::string filepath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name);
	};
}