#include "pepch.h"
#include "Mesh.h"

namespace PaulEngine
{
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
	
	Mesh::Mesh(const MeshSpecification& spec, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices) : m_Spec(spec), m_VertexCount((uint32_t)vertices.size())
	{
		PE_PROFILE_FUNCTION();

		uint32_t indicesCount = (uint32_t)indices.size();
		if (m_Spec.CalculateTangents)
		{
			ComputeTangentsIndexed(&vertices[0], &indices[0], (size_t)m_VertexCount, indices.size());
		}

		m_VertexArray = VertexArray::Create(m_Spec.PrimitiveType);
		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(m_VertexCount * sizeof(MeshVertex), m_Spec.UsageType);

		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float3, "a_Normal", true },
			{ ShaderDataType::Float2, "a_TexCoords", true },
			{ ShaderDataType::Float3, "a_Tangent", true },
			{ ShaderDataType::Float3, "a_Bitangent", true }
		});
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		vertexBuffer->SetData(&vertices[0], sizeof(MeshVertex) * m_VertexCount);
		
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(&indices[0], indicesCount, m_Spec.UsageType);
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}
}