#include "pepch.h"
#include "VertexArrayPool.h"

namespace PaulEngine
{
	void VertexArrayPool::init()
	{
		PE_PROFILE_FUNCTION();

		m_VertexArray = VertexArray::Create();
		m_VertexArray->Bind();

		Ref<VertexBuffer> vbo = VertexBuffer::Create(m_VertexCountCapacity * VertexSize(), BufferUsage::DYNAMIC_DRAW);
		vbo->SetLayout(m_BufferLayout);
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_IndexCountCapacity, BufferUsage::DYNAMIC_DRAW);

		m_VertexArray->AddVertexBuffer(vbo);
		m_VertexArray->SetIndexBuffer(indexBuffer);
		m_VertexArray->Unbind();
	}

	std::optional<std::pair<uint32_t, uint32_t>> VertexArrayPool::RegisterMesh(const void* vertices, const uint32_t vertexCount, const uint32_t* indices, const uint32_t indexCount)
	{
		// Find free block
		const std::optional<size_t> vertexFreeBlockIndex = m_VertexFreeList.FindValidFreeBlockIndex(vertexCount);
		const std::optional<size_t> indexFreeBlockIndex = m_IndexFreeList.FindValidFreeBlockIndex(vertexCount);

		if (!vertexFreeBlockIndex)
		{
			PE_CORE_ERROR("No vertex buffer free block found at requested count: {0}", vertexCount);
			return std::nullopt;
		}

		if (!indexFreeBlockIndex)
		{
			PE_CORE_ERROR("No index buffer free block found at requested count: {0}", indexCount);
			return std::nullopt;
		}

		// Validate buffer sizes
		const FreeBlock& vertexBlock = m_VertexFreeList.GetBlock(vertexFreeBlockIndex.value());
		const uint32_t startMasterVertex = vertexBlock.StartIndex;
		const uint32_t endMasterVertex = startMasterVertex + vertexCount;
		if (endMasterVertex > m_VertexCountCapacity)
		{
			// TODO: Consider dynamic reallocation of master vertex buffer and index buffer to increase size as needed
			PE_CORE_ERROR("Master vertex buffer size reached");
			return std::nullopt;
		}

		const FreeBlock& indexBlock = m_IndexFreeList.GetBlock(indexFreeBlockIndex.value());
		const uint32_t startMasterIndex = indexBlock.StartIndex;
		const uint32_t endMasterIndex = startMasterIndex + indexCount;

		if (endMasterIndex > m_IndexCountCapacity)
		{
			PE_CORE_ERROR("Master index buffer size reached");
			return std::nullopt;
		}

		// Buffer data
		const size_t vertexSize = VertexSize();
		const size_t verticesSize = vertexSize * (size_t)vertexCount;
		const size_t verticesStart = vertexSize * (size_t)startMasterVertex;

		Ref<VertexBuffer> masterVBO = m_VertexArray->GetVertexBuffers()[0];
		masterVBO->SetData(vertices, verticesSize, verticesStart);

		const size_t indicesStart = sizeof(uint32_t) * (size_t)startMasterIndex;

		Ref<IndexBuffer> masterIndexBuffer = m_VertexArray->GetIndexBuffer();
		masterIndexBuffer->SetData(indices, indexCount, indicesStart);

		// Update free list
		bool success = m_VertexFreeList.SplitOrRemoveFreeBlock(vertexFreeBlockIndex.value(), vertexCount);
		if (!success)
		{
			PE_CORE_ERROR("Error updating vertex free list");
			return std::nullopt;
		}

		success = m_IndexFreeList.SplitOrRemoveFreeBlock(indexFreeBlockIndex.value(), indexCount);
		if (!success)
		{
			PE_CORE_ERROR("Error updating index free list");
			return std::nullopt;
		}

		return std::pair<uint32_t, uint32_t>(startMasterVertex, startMasterIndex);
	}
}