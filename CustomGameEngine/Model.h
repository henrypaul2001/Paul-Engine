#pragma once
#include "Mesh.h"
#include "stb_image.h"
#include <map>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
namespace Engine {
	static const unsigned int defaultAssimpPostProcess = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_GenNormals | aiProcess_EmbedTextures;

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

		void RetargetSkeletonRootBone(const std::string& boneName)
		{
			if (bones.find(boneName) != bones.end()) {
				rootBone = &bones[boneName];
			}
			else {
				std::cout << "ERROR::AnimationSkeleton::RetargetSkeletonRootBone::Bone not found" << std::endl;
			}
		}
	};

	class ComponentGeometry;
	class Model
	{
	public:
		Model(const Model& old_model);
		Model(const std::vector<Mesh*>& meshes, bool pbr = false);
		Model(PremadeModel modelType, bool pbr = false);
		~Model();

		void Draw(Shader& shader, int instanceNum, const std::vector<unsigned int> instanceVAOs);
		void DrawTransparentMeshes(Shader& shader, int instanceNum, const std::vector<unsigned int> instanceVAOs);

		bool PBR() { return pbr; }
		void PBR(bool PBR) { pbr = PBR; }

		void ApplyMaterialsToAllMesh(const std::vector<AbstractMaterial*>& materials);
		void ApplyMaterialsToMeshAtIndex(const std::vector<AbstractMaterial*>& materials, const unsigned int index);

		bool ContainsTransparentMeshes() { return containsTransparentMeshes; }

		void SetHasBones(const bool hasBones) { this->hasBones = hasBones; }
		bool HasBones() { return hasBones; }

		void SetAnimationSkeleton(AnimationSkeleton* newSkeleton) { this->skeleton = newSkeleton; }
		AnimationSkeleton* GetAnimationSkeleton() { return skeleton; }

		std::vector<Mesh*> meshes;

		ComponentGeometry* GetOwner() { return owner; }
		const ComponentGeometry* GetOwnerConst() const { return owner; }
		void SetOwner(ComponentGeometry* newOwner) { this->owner = newOwner; }

		void UpdateGeometryBoundingBoxes(glm::mat4 modelMatrix) {
			glm::vec3 scale;
			scale.x = glm::length(glm::vec3(modelMatrix[0]));
			scale.y = glm::length(glm::vec3(modelMatrix[1]));
			scale.z = glm::length(glm::vec3(modelMatrix[2]));

			glm::mat3 rotation;
			rotation[0] = glm::vec3(modelMatrix[0]) / scale.x;
			rotation[1] = glm::vec3(modelMatrix[1]) / scale.y;
			rotation[2] = glm::vec3(modelMatrix[2]) / scale.z;

			// Transform each AABB of each mesh in this model
			for (Mesh* m : meshes) {
				m->GetGeometryAABB().TransformAABB(rotation, scale);
			}
		}
	private:
		std::string directory;
		bool pbr;

		bool containsTransparentMeshes;

		AnimationSkeleton* skeleton;

		bool hasBones;

		ComponentGeometry* owner;
	};
}