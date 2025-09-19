#include "pepch.h"
#include "DrawBatcher.h"

#include "RenderCommand.h"

namespace PaulEngine
{
	size_t DrawBatcher::TotalDrawCount() const
	{
		size_t total = 0;
		for (size_t i = 0; i < m_DrawBatchesInUse; i++)
		{
			total += m_DrawBatches[i].DrawCount();
		}
		return total;
	}

	void DrawBatcher::SubmitMesh(DrawElementsIndirectCommand cmd, MeshSubmissionData meshData, PipelineParams pipelineParams, Ref<Material> materialInstance)
	{
		PE_PROFILE_FUNCTION();
		RenderPipelineSpecification pipelineSpec = { pipelineParams, materialInstance->GetShaderHandle() };

		// Find or create pipeline bin index
		size_t pipelineIndex = FindOrCreatePipelineIndex(pipelineSpec);
		DrawBatch<DRAWS_PER_BATCH>& batch = m_DrawBatches[pipelineIndex];

		// Insert material
		MappedVector<AssetHandle, Ref<Material>>& batchMaterials = batch.GetMaterialInstances();

		std::pair<size_t, bool> materialIndex = batchMaterials.get_index_or_push_back(materialInstance->Handle, materialInstance);
		if (!materialIndex.second)
		{
			// Unique material
			materialInstance->AddBindlessTextureHandlesToSet(&batch.GetDeviceTextureHandles());
		}
		meshData.MaterialIndex = materialIndex.first;

		// Submit
		batch.InsertSubmission(cmd, meshData);
	}

	void DrawBatcher::UploadLocalDrawBuffer(DrawIndirectBuffer* drawBuffer)
	{
		std::vector<DrawIndirectBuffer::DrawIndirectSetDataParams> multiDataParams;
		multiDataParams.reserve(m_DrawBatchesInUse);

		// Collect data pointers from pipeline bins
		uint32_t offset = 0;
		for (DrawBatch<DRAWS_PER_BATCH>& batch : m_DrawBatches)
		{
			// BaseInstance will be used to pass in the running instance count of all previous draw commands to the current draw command so that buffers can be indexed correctly in the shader
			size_t instanceCount = 0;
			std::vector<DrawElementsIndirectCommand>& drawCommands = batch.GetDrawCommands();
			for (DrawElementsIndirectCommand& cmd : drawCommands)
			{
				cmd.BaseInstance = instanceCount;
				instanceCount += cmd.InstanceCount;
			}

			uint32_t drawCount = (uint32_t)batch.DrawCount();
			multiDataParams.push_back({ batch.data(), drawCount, offset });
			offset += drawCount;
		}

		drawBuffer->MultiSetData(multiDataParams, true);
	}

	void DrawBatcher::MakeBatchTexturesResident()
	{
		std::span<const DrawBatch<DRAWS_PER_BATCH>> batches = GetDrawBatches();
		for (auto& batch : batches)
		{
			std::unordered_set<uint64_t> textureSet = batch.GetDeviceTextureHandles();
			for (auto it = textureSet.begin(); it != textureSet.end(); it++)
			{
				RenderCommand::MakeTextureResident(*it);
			}
		}
	}

	void DrawBatcher::MakeBatchTexturesNonResident()
	{
		std::span<const DrawBatch<DRAWS_PER_BATCH>> batches = GetDrawBatches();
		for (auto& batch : batches)
		{
			std::unordered_set<uint64_t> textureSet = batch.GetDeviceTextureHandles();
			for (auto it = textureSet.begin(); it != textureSet.end(); it++)
			{
				RenderCommand::MakeTextureNonResident(*it);
			}
		}
	}

	size_t DrawBatcher::FindOrCreatePipelineIndex(const RenderPipelineSpecification& pipelineSpec)
	{
		std::pair<size_t, bool> pipelineIndex = m_RenderPipelines.get_index_or_emplace_back(pipelineSpec, pipelineSpec);
		if (!pipelineIndex.second)
		{
			// Unique pipeline
			DrawBatch<DRAWS_PER_BATCH>& batch = CreateNewDrawBatch(pipelineIndex.first);
		}
		return pipelineIndex.first;
	}

	DrawBatch<DRAWS_PER_BATCH>& DrawBatcher::CreateNewDrawBatch(size_t drawBatchIndex)
	{
		// Prioritise re-using existing draw batches instead of re-allocating new ones
		DrawBatch<DRAWS_PER_BATCH>& batch = (drawBatchIndex < m_DrawBatches.size()) ? m_DrawBatches[drawBatchIndex] : m_DrawBatches.emplace_back();
		batch.init(drawBatchIndex);
		m_DrawBatchesInUse++;
		return batch;
	}
}