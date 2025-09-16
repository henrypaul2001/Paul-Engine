#pragma once
#include "VertexArray.h"

namespace PaulEngine
{
	struct FreeBlock
	{
		uint32_t StartIndex = 0;
		uint32_t Count = 0;
	};

	struct FreeList
	{
	public:
		FreeList(uint32_t total)
		{
			m_List = { { 0, total } };
		}

		std::vector<FreeBlock>& GetList() { return m_List; }
		const std::vector<FreeBlock>& GetList() const { return m_List; }

		FreeBlock& GetBlock(size_t index) { return m_List[index]; }
		const FreeBlock& GetBlock(size_t index) const { return m_List[index]; }

		std::optional<size_t> FindValidFreeBlockIndex(uint32_t requestedCount)
		{
			for (size_t i = 0; i < m_List.size(); i++)
			{
				if (m_List[i].Count >= requestedCount)
				{
					return i;
				}
			}
			return std::nullopt;
		}

		bool SplitOrRemoveFreeBlock(size_t blockIndex, uint32_t countRemoved)
		{
			if (blockIndex >= m_List.size())
			{
				return false;
			}

			FreeBlock& block = m_List[blockIndex];
			if (block.Count < countRemoved)
			{
				return false;
			}
			
			if (block.Count == countRemoved)
			{
				// Copy the end block into this index and remove the duplicate from the back of the list
				// Avoids a call to erase()
				FreeBlock end = *m_List.end();
				m_List[blockIndex] = end;
				m_List.pop_back();
			}
			else
			{
				// Split block
				block.Count -= countRemoved;
				block.StartIndex += countRemoved;
			}

			return true;
		}

	private:
		std::vector<FreeBlock> m_List;
	};

	class VertexArrayPool
	{
	public:
		VertexArrayPool(const uint32_t vertexCountCapacity, const uint32_t indexCountCapacity, const BufferLayout vertexBufferLayout) : m_VertexCountCapacity(vertexCountCapacity), m_IndexCountCapacity(indexCountCapacity), 
			m_BufferLayout(vertexBufferLayout), m_VertexFreeList(vertexCountCapacity), m_IndexFreeList(indexCountCapacity) {}
		
		void init();

		inline bool is_initialised() const { return (bool)m_VertexArray; }

		inline uint32_t VertexCountCapcity() const { return m_VertexCountCapacity; }
		inline uint32_t IndexCountCapacity() const { return m_IndexCountCapacity; }
		inline size_t VertexSize() const { return m_BufferLayout.GetStride(); }

		const FreeList& GetVertexFreeList() const { return m_VertexFreeList; }
		FreeList& GetVertexFreeList() { return m_VertexFreeList; }

		const FreeList& GetIndexFreeList() const { return m_IndexFreeList; }
		FreeList& GetIndexFreeList() { return m_IndexFreeList; }

		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
		const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }

		// Returns <baseVertexIndex, baseIndicesIndex> if succesfull
		// Returns nullopt if fail
		std::optional<std::pair<uint32_t, uint32_t>> RegisterMesh(const void* vertices, const uint32_t vertexCount, const uint32_t* indices, const uint32_t indexCount);

	private:
		//uint32_t m_CurrentVertexCount = 0;
		//uint32_t m_CurrentIndexCount = 0;
		Ref<VertexArray> m_VertexArray = nullptr;

		const uint32_t m_VertexCountCapacity;
		const uint32_t m_IndexCountCapacity;
		const BufferLayout m_BufferLayout;

		FreeList m_VertexFreeList;
		FreeList m_IndexFreeList;
	};
}