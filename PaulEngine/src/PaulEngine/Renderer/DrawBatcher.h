#pragma once
#include "PaulEngine/Renderer/RenderPipeline.h"
#include "PaulEngine/Renderer/Asset/Material.h"
#include "PaulEngine/Renderer/Resource/ShaderStorageBuffer.h"
#include "PaulEngine/Core/Container/MappedVector.h"

#include <span>

#define DRAWS_PER_BATCH 10000

namespace PaulEngine
{
	template <size_t DRAW_RESERVE_SIZE>
	class DrawBatch
	{
	public:
		DrawBatch()
		{
			m_DrawCommands = std::vector<DrawElementsIndirectCommand>(DRAW_RESERVE_SIZE);
			m_MeshSubmissions = std::vector<MeshSubmissionData>(DRAW_RESERVE_SIZE);
		}

		inline size_t DrawCount() const { return m_DrawCount; }
		inline const DrawElementsIndirectCommand* data() const { return m_DrawCommands.data(); }

		inline const MappedVector<AssetHandle, Ref<Material>>& GetMaterialInstances() const { return m_MaterialInstances; }
		inline MappedVector<AssetHandle, Ref<Material>>& GetMaterialInstances() { return m_MaterialInstances; }

		inline const std::unordered_set<uint64_t>& GetDeviceTextureHandles() const { return m_DeviceTextureHandles; }
		inline std::unordered_set<uint64_t>& GetDeviceTextureHandles() { return m_DeviceTextureHandles; }

		inline const std::vector<MeshSubmissionData>& GetMeshSubmissions() const { return m_MeshSubmissions; }
		inline std::vector<MeshSubmissionData>& GetMeshSubmissions() { return m_MeshSubmissions; }

		inline size_t DrawReserveSize() const { return DRAW_RESERVE_SIZE; }

		void InsertSubmission(DrawElementsIndirectCommand cmd, MeshSubmissionData submission)
		{
			m_DrawCommands[m_DrawCount] = cmd;
			m_MeshSubmissions[m_DrawCount] = submission;
			if (m_DrawCount < DRAW_RESERVE_SIZE)
			{
				m_DrawCount++;
			}
		}

		void init(size_t index)
		{
			m_MaterialInstances.clear();
			m_DeviceTextureHandles.clear();

			m_DrawCount = 0;
			m_Index = index;
		}

	private:
		std::vector<DrawElementsIndirectCommand> m_DrawCommands;
		std::vector<MeshSubmissionData> m_MeshSubmissions;

		MappedVector<AssetHandle, Ref<Material>> m_MaterialInstances;
		std::unordered_set<uint64_t> m_DeviceTextureHandles;

		size_t m_DrawCount = 0;
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