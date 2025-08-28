#include "pepch.h"
#include "Renderer.h"
#include "PaulEngine/Project/Project.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Resource/UniformBuffer.h"
#include "Resource/ShaderStorageBuffer.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "Asset/Material.h"

#include "PaulEngine/Asset/TextureImporter.h"

#define RENDER_TREE_MODE 0

#if RENDER_TREE_MODE
#include "RenderTree.h"
#endif

#define MAX_INDIRECT_DRAW_COMMANDS 1000000
#define PIPELINE_BIN_RESERVE_COUNT (MAX_INDIRECT_DRAW_COMMANDS / 100)

namespace PaulEngine {
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
	};

	static void ComputeTangentsIndexed(QuadVertex* vertices, uint32_t* indices, size_t indexCount, size_t vertexCount) {
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

			QuadVertex& v0 = vertices[i0];
			QuadVertex& v1 = vertices[i1];
			QuadVertex& v2 = vertices[i2];

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

	struct PipelineBin
	{
		std::vector<MeshSubmissionData> MeshSubmissions;
		std::vector<DrawElementsIndirectCommand> DrawCommands;
		size_t DrawCount = 0;
		size_t Index = 0;
	};
	struct PipelineManager
	{
		size_t BinCount() const { return m_PipelineBins.size(); }
		
		const PipelineBin& GetBin(size_t index) const { return m_PipelineBins[index]; }
		const Ref<RenderPipeline> GetPipeline(size_t index) const { return m_RenderPipelines[index]; }
		const std::unordered_map<RenderPipelineHash, size_t>& GetPipelineIndexMap() const { return m_RenderPipelineIndexMap; }

		size_t GetDrawCount() const { return m_TotalDrawCount; }

		void SubmitMesh(DrawElementsIndirectCommand cmd, MeshSubmissionData submissionData, RenderPipelineSpecification pipelineSpec)
		{
			PE_PROFILE_FUNCTION();
			RenderPipelineHash pipelineHash = pipelineSpec.Hash();

			size_t pipelineIndex = FindOrCreatePipelineIndex(pipelineHash, pipelineSpec);

			PipelineBin& bin = m_PipelineBins[pipelineIndex];

			// Submit
			bin.MeshSubmissions.push_back(submissionData);
			bin.DrawCommands.push_back(cmd);
			bin.DrawCount++;
			m_TotalDrawCount++;
		}

		void clear()
		{
			m_PipelineBins.clear();
			m_RenderPipelineIndexMap.clear();
			m_RenderPipelines.clear();
		}

		void UploadLocalDrawBuffer(DrawIndirectBuffer* buffer)
		{
			PE_PROFILE_FUNCTION();

			// Experiment with some other ways to do this without copying potentially large amounts of memory around

			// Join all bin command lists
			uint32_t offset = 0;
			for (size_t i = 0; i < m_PipelineBins.size(); i++)
			{
				PipelineBin& bin = m_PipelineBins[i];
				size_t drawCount = bin.DrawCount;
				std::memcpy(&s_LocalDrawBuffer[offset], bin.DrawCommands.data(), drawCount * sizeof(DrawElementsIndirectCommand));
				bin.DrawCommands.clear();
				bin.DrawCommands.shrink_to_fit();
				offset += drawCount;
			}

			buffer->SetData(s_LocalDrawBuffer.data(), (uint32_t)m_TotalDrawCount, 0, true);
		}

	private:
		size_t FindOrCreatePipelineIndex(const RenderPipelineHash pipelineHash, const RenderPipelineSpecification& pipelineSpec)
		{
			size_t pipelineIndex = 0;

			auto it = m_RenderPipelineIndexMap.find(pipelineHash);
			if (it != m_RenderPipelineIndexMap.end())
			{
				// Duplicate pipeline state
				pipelineIndex = it->second;
			}
			else
			{
				// Unique pipeline
				pipelineIndex = m_PipelineBins.size();
				PipelineBin bin;
				bin.Index = pipelineIndex;
				bin.MeshSubmissions.reserve(PIPELINE_BIN_RESERVE_COUNT);
				bin.DrawCommands.reserve(PIPELINE_BIN_RESERVE_COUNT);

				m_PipelineBins.push_back(bin);
				m_RenderPipelines.push_back(RenderPipeline::Create(pipelineSpec));
				m_RenderPipelineIndexMap[pipelineHash] = pipelineIndex;
			}

			return pipelineIndex;
		}

		std::vector<Ref<RenderPipeline>> m_RenderPipelines;
		std::vector<PipelineBin> m_PipelineBins;
		std::unordered_map<RenderPipelineHash, size_t> m_RenderPipelineIndexMap;
		size_t m_TotalDrawCount = 0;

		static std::vector<DrawElementsIndirectCommand> s_LocalDrawBuffer;
	};
	std::vector<DrawElementsIndirectCommand> PipelineManager::s_LocalDrawBuffer = std::vector<DrawElementsIndirectCommand>(MAX_INDIRECT_DRAW_COMMANDS);

	struct Renderer3DData
	{
		AssetHandle QuadMeshHandle = 0;
		AssetHandle CubeMeshHandle = 0;
		AssetHandle SphereMeshHandle = 0;
		AssetHandle CylinderMeshHandle = 0;
		AssetHandle ConeMeshHandle = 0;
		AssetHandle TorusMeshHandle = 0;

		AssetHandle DefaultLitShaderHandle;
		AssetHandle DefaultLitMaterialHandle;

		AssetHandle DefaultLitPBRShaderHandle;
		AssetHandle DefaultLitPBRMaterialHandle;

		AssetHandle DefaultLitDeferredShaderHandle;
		AssetHandle DefaultLitDeferredMaterialHandle;

		AssetHandle DefaultLitPBRDeferredShaderHandle;
		AssetHandle DefaultLitPBRDeferredMaterialHandle;

		std::unordered_map<AssetHandle, AssetHandle> DefaultForwardToDeferred;
		std::unordered_map<AssetHandle, AssetHandle> DefaultDeferredToForward;

		AssetHandle DefaultForwardMaterial;
		AssetHandle DefaultDeferredMaterial;

		struct CameraData
		{
			glm::mat4 View;
			glm::mat4 Projection;
			glm::vec3 ViewPos;
			float Gamma = 2.2f;
			float Exposure = 1.0f;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		struct SceneMetaData
		{
			int DirLightsHead = 0;
			int PointLightsHead = 0;
			int SpotLightsHead = 0;
		};
		SceneMetaData SceneBufferMetaData;
		struct SceneData
		{
			Renderer::DirectionalLight DirLights[Renderer::MAX_ACTIVE_DIR_LIGHTS];
			Renderer::PointLight PointLights[Renderer::MAX_ACTIVE_POINT_LIGHTS];
			Renderer::SpotLight SpotLights[Renderer::MAX_ACTIVE_SPOT_LIGHTS];
			int ActiveDirLights = 0;
			int ActivePointLights = 0;
			int ActiveSpotLights = 0;
		};
		SceneData SceneDataBuffer;
		Ref<UniformBuffer> SceneDataUniformBuffer;

#if RENDER_TREE_MODE
		RenderTree<Ref<VertexArray>, DepthState, FaceCulling, BlendState, Ref<Material>> RenderTree;
#endif

		Renderer::Statistics Stats;

		PipelineManager PipelineManager;

		Ref<ShaderStorageBuffer> MeshDataBuffer;
		Ref<DrawIndirectBuffer> DrawCommandBuffer;
	};
	static Renderer3DData s_RenderData;

	void Renderer::Init()
	{
		PE_PROFILE_FUNCTION();
		RenderCommand::Init();
		Renderer2D::Init();

		s_RenderData.DrawCommandBuffer = DrawIndirectBuffer::Create(MAX_INDIRECT_DRAW_COMMANDS, StorageBufferMapping::None, true);
		s_RenderData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraBuffer), 0);
		s_RenderData.MeshDataBuffer = ShaderStorageBuffer::Create(sizeof(MeshSubmissionData) * MAX_INDIRECT_DRAW_COMMANDS, 1, StorageBufferMapping::None, true);
		s_RenderData.SceneDataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::SceneDataBuffer), 2);

		s_RenderData.SceneDataBuffer.ActiveDirLights = 0;
		s_RenderData.SceneDataBuffer.ActivePointLights = 0;
		s_RenderData.SceneDataBuffer.ActiveSpotLights = 0;
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;
		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(Renderer3DData::SceneDataBuffer));

#if RENDER_TREE_MODE
		s_RenderData.RenderTree.Init(s_RenderData.MeshDataUniformBuffer);
#endif
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.View = camera.GetViewMatrix();
		s_RenderData.CameraBuffer.Projection = camera.GetProjection();
		s_RenderData.CameraBuffer.ViewPos = camera.GetPosition();
		s_RenderData.CameraBuffer.Gamma = camera.Gamma;
		s_RenderData.CameraBuffer.Exposure = camera.Exposure;
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));

		s_RenderData.SceneDataBuffer.ActiveDirLights = 0;
		s_RenderData.SceneDataBuffer.ActivePointLights = 0;
		s_RenderData.SceneDataBuffer.ActiveSpotLights = 0;
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& worldTransform)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.View = glm::inverse(worldTransform);
		s_RenderData.CameraBuffer.Projection = camera.GetProjection();
		s_RenderData.CameraBuffer.ViewPos = worldTransform[3];
		s_RenderData.CameraBuffer.Gamma = camera.GetGamma();
		s_RenderData.CameraBuffer.Exposure = camera.GetExposure();
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));

		s_RenderData.SceneDataBuffer.ActiveDirLights = 0;
		s_RenderData.SceneDataBuffer.ActivePointLights = 0;
		s_RenderData.SceneDataBuffer.ActiveSpotLights = 0;
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;
	}

	void Renderer::BeginScene(const glm::mat4& projection, const glm::mat4& worldTransform, float gamma, float exposure)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.View = glm::inverse(worldTransform);
		s_RenderData.CameraBuffer.Projection = projection;
		s_RenderData.CameraBuffer.ViewPos = worldTransform[3];
		s_RenderData.CameraBuffer.Gamma = gamma;
		s_RenderData.CameraBuffer.Exposure = exposure;
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));

		s_RenderData.SceneDataBuffer.ActiveDirLights = 0;
		s_RenderData.SceneDataBuffer.ActivePointLights = 0;
		s_RenderData.SceneDataBuffer.ActiveSpotLights = 0;
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;
	}

	void Renderer::EndScene()
	{
		PE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer::Flush()
	{
		PE_PROFILE_FUNCTION();

		s_RenderData.CameraUniformBuffer->Bind(0);

#if !RENDER_TREE_MODE
		s_RenderData.MeshDataBuffer->Bind(1);
#endif

		s_RenderData.SceneDataUniformBuffer->Bind(2);
		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(s_RenderData.SceneDataBuffer));
		
#if RENDER_TREE_MODE
		s_RenderData.Stats.PipelineCount += s_RenderData.RenderTree.MeshBinCount();
		uint16_t drawCalls = s_RenderData.RenderTree.Flush();
		s_RenderData.Stats.DrawCalls += drawCalls;
#else
		Ref<VertexArray> masterVAO = Mesh::GetMasterVAO();
		masterVAO->Bind();
		s_RenderData.DrawCommandBuffer->Bind();
		s_RenderData.PipelineManager.UploadLocalDrawBuffer(s_RenderData.DrawCommandBuffer.get());

		size_t pipelineBinCount = s_RenderData.PipelineManager.BinCount();
		size_t offset = 0;
		for (size_t i = 0; i < pipelineBinCount; i++)
		{
			const PipelineBin& bin = s_RenderData.PipelineManager.GetBin(i);
			const Ref<RenderPipeline> pipeline = s_RenderData.PipelineManager.GetPipeline(i);
			pipeline->Bind();

			s_RenderData.MeshDataBuffer->SetData(bin.MeshSubmissions.data(), bin.MeshSubmissions.size() * sizeof(MeshSubmissionData), 0, true);
			RenderCommand::MultiDrawIndexedIndirect(bin.DrawCount, offset);
			offset += bin.DrawCount;
			s_RenderData.Stats.DrawCalls++;
		}
#endif

		s_RenderData.SceneDataBuffer = Renderer3DData::SceneData();
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;
		
		s_RenderData.Stats.PipelineCount += s_RenderData.PipelineManager.BinCount();
		s_RenderData.PipelineManager.clear();
	}

	void Renderer::SubmitDefaultCube(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		SubmitMesh(s_RenderData.CubeMeshHandle, materialHandle, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::SubmitDefaultQuad(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		SubmitMesh(s_RenderData.QuadMeshHandle, materialHandle, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::SubmitDefaultSphere(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		SubmitMesh(s_RenderData.SphereMeshHandle, materialHandle, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::SubmitMesh(AssetHandle meshHandle, AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		PE_PROFILE_FUNCTION();

		DrawSubmission draw;
		draw.MeshHandle = (AssetManager::IsAssetHandleValid(meshHandle)) ? meshHandle : s_RenderData.SphereMeshHandle;;
		//draw.MaterialHandle = (AssetManager::IsAssetHandleValid(materialHandle)) ? materialHandle : s_RenderData.DefaultForwardMaterial;
		draw.MaterialHandle = s_RenderData.DefaultForwardMaterial;
		draw.Transform = transform;
		draw.EntityID = entityID;

#if RENDER_TREE_MODE
		Ref<Mesh> meshAsset = AssetManager::GetAsset<Mesh>(draw.MeshHandle);
		RenderInput renderInput = { meshAsset->GetVertexArray(), AssetManager::GetAsset<Material>(draw.MaterialHandle), transform, depthState, cullState, blendState, entityID};
		s_RenderData.RenderTree.PushMesh(renderInput);
#else
		Ref<Mesh> meshAsset = AssetManager::GetAsset<Mesh>(draw.MeshHandle);

		MeshSubmissionData submissionData;
		submissionData.Transform = transform;
		submissionData.EntityID = entityID;

		DrawElementsIndirectCommand cmd;
		cmd.Count = meshAsset->NumIndices();
		cmd.InstanceCount = 1;
		cmd.FirstIndex = meshAsset->BaseIndicesIndex();
		cmd.BaseVertex = meshAsset->BaseVertexIndex();
		cmd.BaseInstance = 0;

		s_RenderData.PipelineManager.SubmitMesh(cmd, submissionData, { { cullState, depthState, blendState }, draw.MaterialHandle });
#endif

		s_RenderData.Stats.MeshCount++;
	}

	void Renderer::SubmitDirectionalLightSource(const DirectionalLight& light)
	{
		s_RenderData.SceneDataBuffer.DirLights[s_RenderData.SceneBufferMetaData.DirLightsHead] = light;
		s_RenderData.SceneBufferMetaData.DirLightsHead = ++s_RenderData.SceneBufferMetaData.DirLightsHead % MAX_ACTIVE_DIR_LIGHTS;
		s_RenderData.SceneDataBuffer.ActiveDirLights = std::min(MAX_ACTIVE_DIR_LIGHTS, ++s_RenderData.SceneDataBuffer.ActiveDirLights);
	}

	void Renderer::SubmitPointLightSource(const PointLight& light)
	{
		s_RenderData.SceneDataBuffer.PointLights[s_RenderData.SceneBufferMetaData.PointLightsHead] = light;
		s_RenderData.SceneBufferMetaData.PointLightsHead = ++s_RenderData.SceneBufferMetaData.PointLightsHead % MAX_ACTIVE_POINT_LIGHTS;
		s_RenderData.SceneDataBuffer.ActivePointLights = std::min(MAX_ACTIVE_POINT_LIGHTS, ++s_RenderData.SceneDataBuffer.ActivePointLights);
	}

	void Renderer::SubmitSpotLightSource(const SpotLight& light)
	{
		s_RenderData.SceneDataBuffer.SpotLights[s_RenderData.SceneBufferMetaData.SpotLightsHead] = light;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = ++s_RenderData.SceneBufferMetaData.SpotLightsHead % MAX_ACTIVE_SPOT_LIGHTS;
		s_RenderData.SceneDataBuffer.ActiveSpotLights = std::min(MAX_ACTIVE_SPOT_LIGHTS, ++s_RenderData.SceneDataBuffer.ActiveSpotLights);
	}

	void Renderer::DrawDefaultCubeImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		Ref<Mesh> cubeMesh = AssetManager::GetAsset<Mesh>(s_RenderData.CubeMeshHandle);
		DrawMeshImmediate(cubeMesh, material, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::DrawDefaultQuadImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		Ref<Mesh> quadMesh = AssetManager::GetAsset<Mesh>(s_RenderData.QuadMeshHandle);
		DrawMeshImmediate(quadMesh, material, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::DrawDefaultSphereImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		Ref<Mesh> sphereMesh = AssetManager::GetAsset<Mesh>(s_RenderData.SphereMeshHandle);
		DrawMeshImmediate(sphereMesh, material, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::DrawMeshImmediate(Ref<Mesh> meshAsset, Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		PE_PROFILE_FUNCTION();

		s_RenderData.CameraUniformBuffer->Bind(0);
		s_RenderData.MeshDataBuffer->Bind(1);
		s_RenderData.SceneDataUniformBuffer->Bind(2);

		Ref<RenderPipeline> pipeline = RenderPipeline::Create({ { cullState, depthState, blendState }, 0 });
		pipeline->Bind();
		material->Bind();

		MeshSubmissionData submission;
		submission.Transform = transform;
		submission.EntityID = entityID;
		s_RenderData.MeshDataBuffer->SetData(&submission, sizeof(MeshSubmissionData), 0, false);

		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(Renderer3DData::SceneDataBuffer));

		Mesh::GetMasterVAO()->Bind();
		RenderCommand::DrawIndexedBaseVertex(meshAsset->BaseVertexIndex(), meshAsset->BaseIndicesIndex(), meshAsset->NumIndices());

		s_RenderData.Stats.MeshCount++;
		s_RenderData.Stats.DrawCalls++;
	}

	bool Renderer::SetDefaultMaterial(AssetHandle materialHandle)
	{
		if (AssetManager::IsAssetHandleValid(materialHandle))
		{
			s_RenderData.DefaultForwardMaterial = materialHandle;
			return true;
		}
		PE_CORE_ERROR("Invalid material handle '{0}'", (uint64_t)materialHandle);
		return false;
	}

	bool Renderer::SetDefaultDeferredMaterial(AssetHandle materialHandle)
	{
		if (AssetManager::IsAssetHandleValid(materialHandle))
		{
			s_RenderData.DefaultDeferredMaterial = materialHandle;
			return true;
		}
		PE_CORE_ERROR("Invalid material handle '{0}'", (uint64_t)materialHandle);
		return false;
	}

	AssetHandle Renderer::GetDefaultMaterial()
	{
		return s_RenderData.DefaultForwardMaterial;
	}

	AssetHandle Renderer::GetDefaultDeferredMaterial()
	{
		return s_RenderData.DefaultDeferredMaterial;
	}

	AssetHandle Renderer::GetDefaultLitShader()
	{
		return s_RenderData.DefaultLitShaderHandle;
	}

	AssetHandle Renderer::GetDefaultLitPBRShader()
	{
		return s_RenderData.DefaultLitPBRShaderHandle;
	}

	AssetHandle Renderer::GetDefaultLitDeferredShader()
	{
		return s_RenderData.DefaultLitDeferredShaderHandle;
	}

	AssetHandle Renderer::GetDefaultLitPBRDeferredShader()
	{
		return s_RenderData.DefaultLitPBRDeferredShaderHandle;
	}

	void Renderer::ValidateDefaultShader(AssetHandle& shaderHandle)
	{
		PE_PROFILE_FUNCTION();
		bool isDeferredRenderer = Project::GetRendererContext() == RenderPipelineContext::Deferred;
		if (isDeferredRenderer)
		{
			if (shaderHandle == s_RenderData.DefaultLitShaderHandle || shaderHandle == s_RenderData.DefaultLitPBRShaderHandle)
			{
				shaderHandle = s_RenderData.DefaultForwardToDeferred.at(shaderHandle);
			}
		}
		else
		{
			if (shaderHandle == s_RenderData.DefaultLitDeferredShaderHandle || shaderHandle == s_RenderData.DefaultLitPBRDeferredShaderHandle)
			{
				shaderHandle = s_RenderData.DefaultDeferredToForward.at(shaderHandle);
			}
		}
	}

	void Renderer::ResetStats()
	{
		s_RenderData.Stats.DrawCalls = 0;
		s_RenderData.Stats.MeshCount = 0;
		s_RenderData.Stats.PipelineCount = 0;
	}

	const Renderer::Statistics& Renderer::GetStats()
	{
		return s_RenderData.Stats;
	}

	void Renderer::ImportShaders()
	{
		PE_PROFILE_FUNCTION();
		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		s_RenderData.DefaultLitShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLit.glsl", true);
		s_RenderData.DefaultLitMaterialHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "materials/DefaultLit.pmat", true);

		s_RenderData.DefaultLitPBRShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLitPBR.glsl", true);
		s_RenderData.DefaultLitPBRMaterialHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "materials/DefaultLitPBR.pmat", true);

		s_RenderData.DefaultLitDeferredShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_gBuffer.glsl", true);
		s_RenderData.DefaultLitPBRDeferredShaderHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_gBufferPBR.glsl", true);

		s_RenderData.DefaultLitDeferredMaterialHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "materials/DefaultLitDeferred.pmat", true);
		s_RenderData.DefaultLitPBRDeferredMaterialHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "materials/DefaultLitPBRDeferred.pmat", true);
		
		s_RenderData.DefaultForwardToDeferred = {
			{ s_RenderData.DefaultLitShaderHandle, s_RenderData.DefaultLitDeferredShaderHandle },
			{ s_RenderData.DefaultLitPBRShaderHandle, s_RenderData.DefaultLitPBRDeferredShaderHandle }
		};
		s_RenderData.DefaultDeferredToForward = {
			{ s_RenderData.DefaultLitDeferredShaderHandle, s_RenderData.DefaultLitShaderHandle },
			{ s_RenderData.DefaultLitPBRDeferredShaderHandle, s_RenderData.DefaultLitPBRShaderHandle }
		};

		if (!SetDefaultMaterial(Project::GetActive()->GetSpecification().DefaultMaterial))
		{
			SetDefaultMaterial(s_RenderData.DefaultLitPBRMaterialHandle);
		}
		SetDefaultDeferredMaterial(s_RenderData.DefaultLitPBRDeferredMaterialHandle);

		// Force shader handles to original values regardless of renderer being used
		AssetManager::GetAsset<Material>(s_RenderData.DefaultLitMaterialHandle)->m_ShaderHandle = s_RenderData.DefaultLitShaderHandle;
		AssetManager::GetAsset<Material>(s_RenderData.DefaultLitPBRMaterialHandle)->m_ShaderHandle = s_RenderData.DefaultLitPBRShaderHandle;
		AssetManager::GetAsset<Material>(s_RenderData.DefaultLitDeferredMaterialHandle)->m_ShaderHandle = s_RenderData.DefaultLitDeferredShaderHandle;
		AssetManager::GetAsset<Material>(s_RenderData.DefaultLitPBRDeferredMaterialHandle)->m_ShaderHandle = s_RenderData.DefaultLitPBRDeferredShaderHandle;
	}

	void Renderer::CreateAssets()
	{
		PE_PROFILE_FUNCTION();
		ImportShaders();

		Ref<EditorAssetManager> assetManager = Project::GetActive()->GetEditorAssetManager();
		std::filesystem::path engineAssetsRelativeToProjectAssets = std::filesystem::path("assets").lexically_relative(Project::GetAssetDirectory());

		s_RenderData.QuadMeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultQuad.pmesh", false);
		s_RenderData.CubeMeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultCube.pmesh", false);
		s_RenderData.SphereMeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultSphere.pmesh", false);
		s_RenderData.CylinderMeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultCylinder.pmesh", false);
		s_RenderData.ConeMeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultCone.pmesh", false);
		s_RenderData.TorusMeshHandle = assetManager->ImportAssetFromFile(engineAssetsRelativeToProjectAssets / "models/DefaultTorus.pmesh", false);
	}

	std::string Renderer::ConstructPipelineStateKey(const AssetHandle material, const DepthState depthState, const FaceCulling cullState, const BlendState blendState)
	{
		PE_PROFILE_FUNCTION();
		std::string materialString = std::to_string((uint64_t)material);
		std::string depthString = std::to_string((int)depthState.Func) + std::to_string((int)depthState.Test) + std::to_string((int)depthState.Write);
		std::string cullString = std::to_string((int)cullState);
		std::string blendString = std::to_string((int)blendState.Enabled) + std::to_string((int)blendState.SrcFactor) + std::to_string((int)blendState.DstFactor) + std::to_string((int)blendState.Equation) + 
			std::to_string((float)blendState.ConstantColour.r) + std::to_string((float)blendState.ConstantColour.g) + std::to_string((float)blendState.ConstantColour.b) + std::to_string((float)blendState.ConstantColour.a);

		return materialString + depthString + cullString + blendString;
	}
}