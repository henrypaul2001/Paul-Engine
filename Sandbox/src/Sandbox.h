#pragma once
#include <PaulEngine.h>
#include <PaulEngine/OrthographicCameraController.h>

#include "Maths/Vector.h"

#include <glad/gl.h>

namespace Sandbox
{
#define MAX_MATERIALS 100
#define INITIAL_BUFFER_CAPACITY_IN_BYTES 56000000
#define INITIAL_INDEX_CAPACITY_COUNT 1000000
	// 56 million bytes = 1,000,000 vertices
#define MAX_DRAW_COMMANDS 1000000

	struct BasicMaterial
	{
		glm::vec4 Albedo;
		glm::vec4 Specular;

		glm::vec3 EmissionColour;
		float EmissionStrength;

		glm::vec2 TextureScale;
		float Shininess;
		//float HeightScale;
		int AlbedoTextureIndex;

		glm::vec2 padding0 = glm::vec2(0.0f);
		int SpecularTextureIndex;
		int NormalTextureIndex = -1;
		//int UseDisplacementMap;
	};

	struct DrawElementsIndirectCommand
	{
		unsigned int count = 0;
		unsigned int instanceCount = 0;
		unsigned int firstIndex = 0;
		int baseVertex = 0;
		unsigned int baseInstance = 0;
	};

	struct MeshSubmissionData
	{
		glm::mat4 Transform;
		int EntityID;
		unsigned int MaterialID;
		int padding1;
		int padding2;
	};

	struct BatchedMesh
	{
		uint32_t BaseVertexIndex;
		uint32_t BaseIndicesIndex;
		uint32_t NumIndices;
	};

	// Potential way of tracking freed memory
	/*
	struct FreeMemory
	{
		size_t StartPosBytes;
		size_t SizeBytes;
	}

	BatchedMeshManager
	{
		std::vector<FreeMemory> m_FreeMemory;
	}
	When registering a mesh, iterate the list of free memory to find a slot of memory that the mesh can fit into
	When "deallocating" a mesh, create a new instance of FreeMemory representing where that mesh used to be in the
	buffer and add to the free memory list
	*/

	class BatchedMeshManager
	{
	public:
		BatchedMeshManager()
		{
			m_MasterVAO = PaulEngine::VertexArray::Create();
			m_MasterVertexBuffer = PaulEngine::VertexBuffer::Create(INITIAL_BUFFER_CAPACITY_IN_BYTES, PaulEngine::BufferUsage::DYNAMIC_DRAW);
			m_MasterVertexBuffer->SetLayout({
				{ PaulEngine::ShaderDataType::Float3, "a_Position", false },
				{ PaulEngine::ShaderDataType::Float3, "a_Normal", true },
				{ PaulEngine::ShaderDataType::Float2, "a_TexCoords", true },
				{ PaulEngine::ShaderDataType::Float3, "a_Tangent", true },
				{ PaulEngine::ShaderDataType::Float3, "a_Bitangent", true }
			});
			m_MasterIndexBuffer = PaulEngine::IndexBuffer::Create(INITIAL_INDEX_CAPACITY_COUNT, PaulEngine::BufferUsage::DYNAMIC_DRAW);
			m_MasterVAO->AddVertexBuffer(m_MasterVertexBuffer);
			m_MasterVAO->SetIndexBuffer(m_MasterIndexBuffer);
		}

		PaulEngine::Ref<PaulEngine::VertexArray>& GetVertexArray() { return m_MasterVAO; }

		BatchedMesh RegisterLoadedMesh(std::vector<PaulEngine::MeshVertex>& vertices, std::vector<uint32_t>& indices)
		{
			// Upload vertices and indices
			const size_t verticesSizeInBytes = sizeof(PaulEngine::MeshVertex) * vertices.size();
			const size_t verticesStartMemoryPosition = m_CurrentVertexBufferSize;
			const size_t verticesEndMemoryPosition = verticesStartMemoryPosition + verticesSizeInBytes;

			PE_ASSERT(verticesEndMemoryPosition <= INITIAL_BUFFER_CAPACITY_IN_BYTES, "Vertex buffer overrun");

			const size_t indicesSizeInBytes = sizeof(uint32_t) * indices.size();
			const size_t indicesStartMemoryPosition = m_CurrentIndexBufferSize;
			const size_t indicesEndMemoryPosition = indicesStartMemoryPosition + indicesSizeInBytes;

			PE_ASSERT(indicesEndMemoryPosition <= INITIAL_INDEX_CAPACITY_COUNT * sizeof(uint32_t), "Index buffer overrun");

			m_MasterVertexBuffer->SetData(&vertices[0], verticesSizeInBytes, verticesStartMemoryPosition);
			m_CurrentVertexBufferSize = verticesEndMemoryPosition;

			m_MasterIndexBuffer->SetData(&indices[0], indices.size(), indicesStartMemoryPosition);
			m_CurrentIndexBufferSize = indicesEndMemoryPosition;

			BatchedMesh registeredMesh;
			registeredMesh.BaseVertexIndex = verticesStartMemoryPosition / sizeof(PaulEngine::MeshVertex);
			registeredMesh.BaseIndicesIndex = indicesStartMemoryPosition / sizeof(uint32_t);
			registeredMesh.NumIndices = indices.size();

			return registeredMesh;
		}

	private:
		size_t m_CurrentVertexBufferSize = 0;
		size_t m_CurrentIndexBufferSize = 0;

		PaulEngine::Ref<PaulEngine::VertexArray> m_MasterVAO;
		PaulEngine::Ref<PaulEngine::VertexBuffer> m_MasterVertexBuffer;
		PaulEngine::Ref<PaulEngine::IndexBuffer> m_MasterIndexBuffer;
	};

	class Sandbox : public PaulEngine::Layer
	{
	public:
		Sandbox();
		~Sandbox();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(const PaulEngine::Timestep timestep) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(PaulEngine::Event& e) override;

	private:
		bool OnKeyUp(PaulEngine::KeyReleasedEvent& e);
		bool OnViewportResize(PaulEngine::MainViewportResizeEvent& e);

		BatchedMeshManager m_MeshManager;
		PaulEngine::Ref<PaulEngine::EditorCamera> m_Camera;

		std::vector<BatchedMesh> m_MeshList;
		std::vector<glm::mat4> m_MeshTransforms;
		std::vector<uint32_t> m_MaterialIDs;

		PaulEngine::Ref<PaulEngine::Shader> m_Shader;

		uint32_t m_ViewportWidth, m_ViewportHeight;

		std::vector<DrawElementsIndirectCommand> m_LocalCommandsBuffer;
		uint32_t m_DrawCommandBufferID;
		size_t m_DrawCommandBufferSize;

		std::vector<MeshSubmissionData> m_LocalMeshSubmissionBuffer;
		uint32_t m_MeshSubmissionBufferID;
		size_t m_MeshSubmissionBufferSize;

		std::vector<BasicMaterial> m_LocalMaterialBuffer;
		uint32_t m_MaterialBufferID;
		size_t m_MaterialBufferSize;

		std::vector<PaulEngine::Ref<PaulEngine::Texture2D>> m_Textures;

		std::vector<GLuint64> m_LocalTextureBuffer;
		uint32_t m_TextureBufferID;

		std::unordered_set<GLuint64> m_TextureSubmissions;
	};
}