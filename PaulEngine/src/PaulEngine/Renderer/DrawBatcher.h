#pragma once
#include "PaulEngine/Renderer/RenderPipeline.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Renderer/Resource/ShaderStorageBuffer.h"
#include "PaulEngine/Core/Container/MappedVector.h"

#include <span>

#define DRAWS_PER_BATCH 10000

namespace PaulEngine
{
	struct DrawKey
	{
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t IndexCount;

		bool operator==(const DrawKey& other) const {
			return (
				BaseVertex == other.BaseVertex &&
				BaseIndex == other.BaseIndex &&
				IndexCount == other.IndexCount
			);
		}
	};

}
MAKE_HASHABLE(PaulEngine::DrawKey, t.BaseVertex, t.BaseIndex, t.IndexCount)

namespace PaulEngine
{
	template <size_t DRAW_RESERVE_SIZE>
	class DrawBatch
	{
	public:
		DrawBatch()
		{
			//m_DrawCommands = std::vector<DrawElementsIndirectCommand>(DRAW_RESERVE_SIZE);
			//m_MeshSubmissions = std::vector<MeshSubmissionData>(DRAW_RESERVE_SIZE);
			m_DrawCommands.reserve(DRAW_RESERVE_SIZE);
			m_SingleInstanceMeshSubmissions.reserve(DRAW_RESERVE_SIZE);
			m_InstancedMeshSubmissions.reserve(10);
		}

		inline size_t DrawCount() const { return m_DrawCommands.size(); }
		inline size_t SubmissionCount() const { return m_SubmissionCount; }
		inline const DrawElementsIndirectCommand* data() const { return m_DrawCommands.data(); }

		inline const std::vector<DrawElementsIndirectCommand>& GetDrawCommands() const { return m_DrawCommands; }
		inline std::vector<DrawElementsIndirectCommand>& GetDrawCommands() { return m_DrawCommands; }

		inline const MappedVector<AssetHandle, Ref<Material>>& GetMaterialInstances() const { return m_MaterialInstances; }
		inline MappedVector<AssetHandle, Ref<Material>>& GetMaterialInstances() { return m_MaterialInstances; }

		inline const std::unordered_set<uint64_t>& GetDeviceTextureHandles() const { return m_DeviceTextureHandles; }
		inline std::unordered_set<uint64_t>& GetDeviceTextureHandles() { return m_DeviceTextureHandles; }

		inline size_t DrawReserveSize() const { return DRAW_RESERVE_SIZE; }

		void UploadLocalMeshSubmissions(ShaderStorageBuffer* storageBuffer) const
		{
			std::vector<ShaderStorageBuffer::SetDataParams> multiDataParams;
			multiDataParams.reserve(1 + m_InstancedMeshSubmissions.size());
			
			const size_t meshSubmissionSize = sizeof(MeshSubmissionData);

			// Collect data pointers
			uint32_t offset = 0;
			if (m_SingleInstanceMeshSubmissions.size() > 0)
			{
				multiDataParams.push_back({ m_SingleInstanceMeshSubmissions.data(), m_SingleInstanceMeshSubmissions.size() * meshSubmissionSize, offset * meshSubmissionSize });
				offset += m_SingleInstanceMeshSubmissions.size();
			}

			for (const std::vector<MeshSubmissionData>& buffer : m_InstancedMeshSubmissions)
			{
				uint32_t count = (uint32_t)buffer.size();
				multiDataParams.push_back({ buffer.data(), count * meshSubmissionSize, offset * meshSubmissionSize });
				offset += count;
			}
			
			storageBuffer->MultiSetData(multiDataParams, true);
		}

		/*
		Keep instanced commands at the end of the drawCommands buffer
		drawBuffer = { 0, 1, 2, 3, 4, 5,        6, 7  }
						non instanced        instanced
		In above example, if draw command 1 was now updated to be instanced (a second instance of the same mesh was submitted), do the following:
			swap draw command 1 with draw command 5
			swap meshSubmission 1 with meshSubmission 5 in singleInstancedMeshSubmissions and pop back() to remove meshSubmission 1 efficiently
			add meshSubmission 1 into a new vector inside of instancedMeshSubmissions (create a new instanced submission buffer) and then append the newly created submission from original call to Submit() to the newly created instanced buffer

		Draw commands can be buffered in the same way as before, and the mesh submissions would need to be multi buffered as a collection of data pointers
		This implementation will make sure that each draw command correctly aligns with its mesh submission (once the mesh submission buffer has been created on the GPU)
		*/

		void InsertSubmission(DrawElementsIndirectCommand cmd, MeshSubmissionData submission)
		{
			DrawKey key = { cmd.BaseVertex, cmd.FirstIndex, cmd.Count };
			auto it = m_DrawCommandMap.find(key);
			if (it != m_DrawCommandMap.end())
			{
				// Duplicate key
				size_t drawIndex = it->second;

				if (m_DrawCommands[drawIndex].InstanceCount == 1)
				{
					// Transform single instance draw into instanced draw

					size_t newDrawIndex = m_SingleInstanceMeshSubmissions.size() - 1; // 4

					// Swap current cmd with back cmd in single instance region
					DrawElementsIndirectCommand backCmdCopy = m_DrawCommands[newDrawIndex];
					m_DrawCommands[newDrawIndex] = cmd;
					m_DrawCommands[drawIndex] = backCmdCopy;

					// Update draw cmd indices
					// We don't need to update the indices of the following instanced cmds as no new commands have been inserted into the buffer
					DrawKey movedKey = { backCmdCopy.BaseVertex, backCmdCopy.FirstIndex, backCmdCopy.Count };
					m_DrawCommandMap[movedKey] = drawIndex;
					m_DrawCommandMap[key] = newDrawIndex;

					MeshSubmissionData transformedDrawCopy = m_SingleInstanceMeshSubmissions[drawIndex];

					// Efficiently remove element by swapping with back element and popping back
					MeshSubmissionData backCopy = m_SingleInstanceMeshSubmissions.back();
					m_SingleInstanceMeshSubmissions[drawIndex] = backCopy;
					m_SingleInstanceMeshSubmissions.pop_back();

					// TODO: avoid allocating buffers every frame, reserve on DrawBatch construction instead and keep track of m_InstancedMeshSubmissions.size() manually
					std::vector<MeshSubmissionData> newInstancedBuffer;
					newInstancedBuffer.reserve(DRAW_RESERVE_SIZE);

					newInstancedBuffer.push_back(transformedDrawCopy);
					newInstancedBuffer.push_back(submission);
					m_InstancedMeshSubmissions.insert(m_InstancedMeshSubmissions.begin(), newInstancedBuffer);

					m_DrawCommands[newDrawIndex].InstanceCount++;
				}
				else
				{
					// Simple append of instanced draw
					m_DrawCommands[drawIndex].InstanceCount++;
					size_t instancedBufferIndex = drawIndex - m_SingleInstanceMeshSubmissions.size();
					m_InstancedMeshSubmissions[instancedBufferIndex].push_back(submission);
				}
			}
			else
			{
				// Insertion will have to work a bit differently
				// Insert at the end of the single instance command region, before the instanced command region (shared buffer)
				// Iterate over the following commands (will be the instanced commands) and update their index in the command map as they have now been shifted by one

				// New key
				size_t drawIndex = m_DrawCommands.size() - m_InstancedMeshSubmissions.size();
				m_DrawCommandMap[key] = drawIndex;
				if (m_InstancedMeshSubmissions.size() == 0)
				{
					m_DrawCommands.push_back(cmd);
				}
				else
				{
					m_DrawCommands.insert(m_DrawCommands.begin() + drawIndex, cmd);
				}
				m_SingleInstanceMeshSubmissions.push_back(submission);

				// Update instanced draw indices
				for (size_t i = drawIndex + 1; i < m_DrawCommands.size(); i++)
				{
					DrawElementsIndirectCommand& cmd = m_DrawCommands[i];
					DrawKey key = { cmd.BaseVertex, cmd.FirstIndex, cmd.Count };
					m_DrawCommandMap[key] = i;
				}
			}
			
			m_SubmissionCount++;
		}

		void init(size_t index)
		{
			m_MaterialInstances.clear();
			m_DeviceTextureHandles.clear();
			m_DrawCommands.clear();
			m_DrawCommandMap.clear();

			m_SingleInstanceMeshSubmissions.clear();
			m_InstancedMeshSubmissions.clear();

			m_SubmissionCount = 0;
			m_Index = index;
		}

	private:
		std::unordered_map<DrawKey, size_t> m_DrawCommandMap; // <commandKey, index>
		std::vector<DrawElementsIndirectCommand> m_DrawCommands;

		std::vector<MeshSubmissionData> m_SingleInstanceMeshSubmissions;
		std::vector<std::vector<MeshSubmissionData>> m_InstancedMeshSubmissions;

		MappedVector<AssetHandle, Ref<Material>> m_MaterialInstances;
		std::unordered_set<uint64_t> m_DeviceTextureHandles;

		size_t m_SubmissionCount = 0;
		size_t m_Index = 0;
	};

	class DrawBatcher
	{
	public:
		DrawBatcher()
		{
			for (size_t i = 0; i < m_DrawBatches.size(); i++)
			{
				m_DrawBatches[i].init(i);
			}
		}

		const MappedVector<RenderPipelineSpecification, RenderPipeline>& GetPipelines() const { return m_RenderPipelines; }
		const std::span<const DrawBatch<DRAWS_PER_BATCH>, std::dynamic_extent> GetDrawBatches() const
		{
			std::span<const DrawBatch<DRAWS_PER_BATCH>, std::dynamic_extent> span = std::span<const DrawBatch<DRAWS_PER_BATCH>>(m_DrawBatches);
			return span.subspan(0, m_DrawBatchesInUse);
		}

		size_t TotalDrawCount() const;
		inline size_t BatchCount() const { return m_DrawBatchesInUse; }

		void SubmitMesh(DrawElementsIndirectCommand cmd, MeshSubmissionData meshData, PipelineParams pipelineParams, Ref<Material> materialInstance);

		// TODO: Investigate a single local draw command buffer instead of each mesh bin having their own local buffer
		void UploadLocalDrawBuffer(DrawIndirectBuffer* drawBuffer);

		void MakeBatchTexturesResident();
		void MakeBatchTexturesNonResident();

		void Reset()
		{
			m_DrawBatchesInUse = 0;
			m_RenderPipelines.clear();
		}

	private:
		size_t FindOrCreatePipelineIndex(const RenderPipelineSpecification& pipelineSpec);
		DrawBatch<DRAWS_PER_BATCH>& CreateNewDrawBatch(size_t drawBatchIndex);

		MappedVector<RenderPipelineSpecification, RenderPipeline> m_RenderPipelines;

		// The draw batches vector will not reserve an initial capacity, there could only be 1 pipeline ever used in an application
		// so no need to allocate more than that in that case (waste of potentially a lot of memory depending on max draw commands)

		// When a new pipeline is needed, increase capacity as needed

		// When the batcher is reset, memory will stay allocated and DrawBatchesInUse will reset to 0
		// Allowing existing draw batches to be re-used instead of constantly reallocated every frame

		std::vector<DrawBatch<DRAWS_PER_BATCH>> m_DrawBatches;
		size_t m_DrawBatchesInUse = 0;
	};
}