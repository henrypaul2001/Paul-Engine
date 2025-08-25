#include "pepch.h"
#include "Mesh.h"

namespace PaulEngine
{
	Ref<VertexArray> Mesh::s_MasterVertexArray = nullptr;
	uint32_t Mesh::s_CurrentMasterVertexCount = 0;
	uint32_t Mesh::s_CurrentMasterIndexCount = 0;

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

		if (!s_MasterVertexArray) { InitMasterVAO(); }

		uint32_t indicesCount = (uint32_t)indices.size();
		if (m_Spec.CalculateTangents)
		{
			ComputeTangentsIndexed(&vertices[0], &indices[0], (size_t)m_VertexCount, indices.size());
		}

		bool success = RegisterToMaster(this, vertices, indices);
		PE_CORE_ASSERT(success, "Error registering mesh");
	}

	void Mesh::InitMasterVAO()
	{
		PE_PROFILE_FUNCTION();
		s_MasterVertexArray = VertexArray::Create();
		s_MasterVertexArray->Bind();

		Ref<VertexBuffer> vbo = VertexBuffer::Create(INITIAL_MASTER_VERTEX_COUNT * sizeof(MeshVertex), BufferUsage::DYNAMIC_DRAW);
		vbo->SetLayout({
			{ ShaderDataType::Float3, "a_Position",  false },
			{ ShaderDataType::Float3, "a_Normal",    true  },
			{ ShaderDataType::Float2, "a_TexCoords", true  },
			{ ShaderDataType::Float3, "a_Tangent",	 true  },
			{ ShaderDataType::Float3, "a_Bitangent", true  }
		});
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(INITIAL_MASTER_INDEX_COUNT, BufferUsage::DYNAMIC_DRAW);

		s_MasterVertexArray->AddVertexBuffer(vbo);
		s_MasterVertexArray->SetIndexBuffer(indexBuffer);
		s_MasterVertexArray->Unbind();
	}

	bool Mesh::RegisterToMaster(Mesh* m, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices)
	{
		// Validate buffer sizes

		const uint32_t vertexCount = m->m_VertexCount;
		const uint32_t startMasterVertex = s_CurrentMasterVertexCount;
		const uint32_t endMasterVertex = startMasterVertex + vertexCount;

		if (endMasterVertex > INITIAL_MASTER_VERTEX_COUNT)
		{
			// TODO: Consider dynamic reallocation of master vertex buffer and index buffer to increase size as needed
			PE_CORE_ERROR("Master vertex buffer size reached");
			return false;
		}

		const uint32_t indexCount = m->m_IndexCount;
		const uint32_t startMasterIndex = s_CurrentMasterIndexCount;
		const uint32_t endMasterIndex = startMasterIndex + indexCount;

		if (endMasterIndex > INITIAL_MASTER_INDEX_COUNT)
		{
			PE_CORE_ERROR("Master index buffer size reached");
			return false;
		}
		
		// Buffer data

		const size_t verticesSize = sizeof(MeshVertex) * (size_t)vertexCount;
		const size_t verticesStart = sizeof(MeshVertex) * (size_t)startMasterVertex;

		Ref<VertexBuffer> masterVBO = s_MasterVertexArray->GetVertexBuffers()[0];
		masterVBO->SetData(vertices.data(), verticesSize, verticesStart);
		s_CurrentMasterVertexCount = endMasterVertex;

		const size_t indicesStart = sizeof(uint32_t) * (size_t)startMasterIndex;

		Ref<IndexBuffer> masterIndexBuffer = s_MasterVertexArray->GetIndexBuffer();
		masterIndexBuffer->SetData(indices.data(), indexCount, indicesStart);
		s_CurrentMasterIndexCount = endMasterIndex;

		m->m_BaseVertexIndex = startMasterVertex;
		m->m_BaseIndicesIndex = startMasterIndex;

		return true;
	}
}