#include "MeshData.h"
namespace Engine {
	MeshData::MeshData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	{
		this->vertices = vertices;
		this->indices = indices;
		SetupMesh();
	}

	MeshData::~MeshData()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &SSBO);
	}

	void MeshData::SetupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		// ids
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));

		// weights
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneWeights));
		glBindVertexArray(0);

		// 7, 8, 9, 10 reserved for instancing

		// SSBO for animated AABB calculations
		glGenBuffers(1, &SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);

		const unsigned int numVertices = vertices.size();
		std::vector<GPUComputeVertex> gpuVertices;
		gpuVertices.reserve(numVertices);
		for (unsigned int i = 0; i < numVertices; i++) {
			const Vertex& v = vertices[i];
			gpuVertices.push_back(GPUComputeVertex(glm::vec4(v.Position, 1.0f), v.BoneIDs, v.BoneWeights));
		}
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUComputeVertex) * numVertices, &gpuVertices[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}