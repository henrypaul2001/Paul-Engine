#pragma once
#include "PaulEngine/Renderer/Resource/VertexArrayPool.h"
#include "PaulEngine/Asset/Asset.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

namespace PaulEngine
{
	// Distribution builds should optimise these values according to the assets built into the distribution asset packs

#define INITIAL_MASTER_VERTEX_COUNT 1000000 // 1 million
#define INITIAL_MASTER_INDEX_COUNT 10000000 // 10 million

	struct MeshVertex
	{
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f);
		glm::vec2 TexCoords = glm::vec2(0.0f);
		glm::vec3 Tangent = glm::vec3(0.0f);
		glm::vec3 Bitangent = glm::vec3(0.0f);
	};

	static void ComputeTangentsIndexed(MeshVertex* vertices, uint32_t* indices, size_t vertexCount, size_t indexCount);

	struct MeshSpecification
	{
		bool CalculateTangents = false;
		std::string Name = "Mesh";
	};

	class Mesh : public Asset
	{
	public:
		Mesh(const MeshSpecification& spec, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices);
		~Mesh()
		{
			s_MasterVertexArray.UnregisterMesh(m_BaseVertexIndex, m_VertexCount, m_BaseIndicesIndex, m_IndexCount);
		}

		virtual AssetType GetType() const override { return AssetType::Mesh; }

		const MeshSpecification& GetSpec() const { return m_Spec; }

		uint32_t NumVertices() const { return m_VertexCount; }
		uint32_t NumIndices() const { return m_IndexCount; }
		uint32_t BaseVertexIndex() const { return m_BaseVertexIndex; }
		uint32_t BaseIndicesIndex() const { return m_BaseIndicesIndex; }

		static Ref<VertexArray> GetMasterVAO() {
			if (!s_MasterVertexArray.is_initialised()) { s_MasterVertexArray.init(); }
			return s_MasterVertexArray.GetVertexArray();
		}

		static const VertexArrayPool& GetMasterVertexArrayPool() { return s_MasterVertexArray; }

	private:
		MeshSpecification m_Spec;

		uint32_t m_VertexCount = 0;
		uint32_t m_IndexCount = 0;

		uint32_t m_BaseVertexIndex = 0;
		uint32_t m_BaseIndicesIndex = 0;

		static VertexArrayPool s_MasterVertexArray;
	};

	class Model : public Asset
	{
	public:
		Model() {}
		virtual AssetType GetType() const override { return AssetType::Model; }

		const size_t NumMeshes() const { return m_Meshes.size(); }
		Ref<Mesh> GetMesh(uint32_t index) { PE_CORE_ASSERT(index < m_Meshes.size(), "Index out of bounds"); return m_Meshes[index]; }

	private:
		friend class MeshImporter;

		std::vector<Ref<Mesh>> m_Meshes = {};
	};
}