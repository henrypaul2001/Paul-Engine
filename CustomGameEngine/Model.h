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
		// Map of bones attached to a mesh
		std::map<std::string, AnimationBone> bones;

		// Map of bones that are not tied to any mesh or vertex, but required to apply transformations to the rest of the bones
		std::map<std::string, AnimationBone> emptyBones;

		AnimationBone* rootBone;
		glm::mat4 originTransform;

		std::vector<glm::mat4> finalBoneMatrices;
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

		bool HasBones() { return hasBones; }

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

		AnimationSkeleton& GetAnimationSkeleton() { return skeleton; }

		void RetargetSkeletonRootBone(const std::string& boneName);

		std::vector<Mesh*> meshes;
		//std::vector<Texture> textures_loaded;
	private:
		std::string directory;
		bool pbr;

		bool containsTransparentMeshes;

		AnimationSkeleton skeleton;

		bool hasBones;

		void LoadModel(std::string filepath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
		void ProcessBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
		bool ProcessEmptyBones(aiNode* node);
		std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureTypes name);
	};
}