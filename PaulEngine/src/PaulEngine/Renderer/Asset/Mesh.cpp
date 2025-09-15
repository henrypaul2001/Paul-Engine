#include "pepch.h"
#include "Mesh.h"

namespace PaulEngine
{
	VertexArrayPool Mesh::s_MasterVertexArray = VertexArrayPool(INITIAL_MASTER_VERTEX_COUNT, INITIAL_MASTER_INDEX_COUNT, {
			{ ShaderDataType::Float3, "a_Position",  false },
			{ ShaderDataType::Float3, "a_Normal",    true  },
			{ ShaderDataType::Float2, "a_TexCoords", true  },
			{ ShaderDataType::Float3, "a_Tangent",	 true  },
			{ ShaderDataType::Float3, "a_Bitangent", true  }
		});

	void ComputeTangentsIndexed(MeshVertex* vertices, uint32_t* indices, size_t vertexCount, size_t indexCount)
	{
		PE_PROFILE_FUNCTION();
		// Initialize accumulators
		for (size_t i = 0; i < vertexCount; i++)
		{
			vertices[i].Tangent = glm::vec3(0.0f);
			vertices[i].Bitangent = glm::vec3(0.0f);
		}

		for (size_t i = 0; i < indexCount; i += 3)
		{
			uint32_t i0 = indices[i + 0];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			MeshVertex& v0 = vertices[i0];
			MeshVertex& v1 = vertices[i1];
			MeshVertex& v2 = vertices[i2];

			glm::vec3 edge1 = v1.Position - v0.Position;
			glm::vec3 edge2 = v2.Position - v0.Position;
			glm::vec2 deltaUV1 = v1.TexCoords - v0.TexCoords;
			glm::vec2 deltaUV2 = v2.TexCoords - v0.TexCoords;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
			glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

			// Accumulate
			v0.Tangent += tangent;
			v1.Tangent += tangent;
			v2.Tangent += tangent;

			v0.Bitangent += bitangent;
			v1.Bitangent += bitangent;
			v2.Bitangent += bitangent;
		}

		// Normalize all accumulated tangents and bitangents
		for (size_t i = 0; i < vertexCount; i++)
		{
			vertices[i].Tangent = glm::normalize(vertices[i].Tangent);
			vertices[i].Bitangent = glm::normalize(vertices[i].Bitangent);
		}
	}
	
	Mesh::Mesh(const MeshSpecification& spec, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices) : m_Spec(spec), m_VertexCount((uint32_t)vertices.size()), m_IndexCount((uint32_t)indices.size())
	{
		PE_PROFILE_FUNCTION();

		if (!s_MasterVertexArray.is_initialised()) { s_MasterVertexArray.init(); }

		uint32_t indicesCount = (uint32_t)indices.size();
		if (m_Spec.CalculateTangents)
		{
			ComputeTangentsIndexed(&vertices[0], &indices[0], (size_t)m_VertexCount, indices.size());
		}

		std::optional<std::pair<uint32_t, uint32_t>> success = s_MasterVertexArray.RegisterMesh((const void*)vertices.data(), m_VertexCount, indices.data(), m_IndexCount);
		PE_CORE_ASSERT(success, "Error registering mesh");

		if (success)
		{
			m_BaseVertexIndex = success.value().first;
			m_BaseIndicesIndex = success.value().second;
		}
	}
}