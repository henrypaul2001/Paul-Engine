#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <vector>
#include <glad/glad.h>
namespace Engine {
	static constexpr int MaxBoneInfluence() { return 8; }
	static constexpr int MaxBoneCount() { return 200; }

	struct GPUComputeVertex {
		GPUComputeVertex(glm::vec4 pos, const int* bones, const float* weights) : positionLocalSpace(pos) {
			for (unsigned int i = 0; i < 8; i++) {
				boneIDs[i] = bones[i];
				boneWeights[i] = weights[i];
			}
		}

		glm::vec4 positionLocalSpace;
		int boneIDs[8];
		float boneWeights[8];
	};

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		int BoneIDs[MaxBoneInfluence()] = { -1, -1, -1, -1, -1, -1, -1, -1 };
		float BoneWeights[MaxBoneInfluence()] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(int boneID, float boneWeight) {
			if (boneWeight == 0.0f) {
				// skip bone
				return;
			}

			for (int i = 0; i < MaxBoneInfluence(); i++) {
				if (BoneIDs[i] == -1 && BoneIDs[i] != boneID) {
					// empty bone slot found
					BoneIDs[i] = boneID;
					BoneWeights[i] = boneWeight;
					return;
				}
			}
		}
	};

	class MeshData
	{
	public:
		MeshData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
		~MeshData();

		void DrawMeshData(const unsigned int instanceNum, const GLenum drawPrimitive, const unsigned int instanceVAO = 0) {
			if (instanceNum == 0) {
				glBindVertexArray(VAO);
				glDrawElements(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
			}
			else if (instanceNum > 0) {
				glBindVertexArray(instanceVAO);
				glDrawElementsInstanced(drawPrimitive, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceNum);
			}
			glBindVertexArray(0);
		}

		const std::vector<Vertex>& GetVertices() const { return vertices; }
		const unsigned int GetVAO() const { return VAO; }
		const unsigned int GetVBO() const { return VBO; }
		const unsigned int GetEBO() const { return EBO; }
		const unsigned int GetSSBO() const { return SSBO; }
	private:
		void SetupMesh();

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int VAO, VBO, EBO;
		unsigned int SSBO;
	};
}