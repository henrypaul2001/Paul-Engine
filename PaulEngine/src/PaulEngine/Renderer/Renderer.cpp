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

#include "PaulEngine/Renderer/DrawBatcher.h"

#define MAX_INDIRECT_DRAW_COMMANDS 10000

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

		Renderer::Statistics Stats;

		DrawBatcher DrawBatcher;

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
		s_RenderData.MeshDataBuffer->Bind(1);

		s_RenderData.SceneDataUniformBuffer->Bind(2);
		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(s_RenderData.SceneDataBuffer));

		Ref<VertexArray> masterVAO = Mesh::GetMasterVAO();
		masterVAO->Bind();
		s_RenderData.DrawCommandBuffer->Bind();

		DrawBatcher& batcher = s_RenderData.DrawBatcher;

		batcher.UploadLocalDrawBuffer(s_RenderData.DrawCommandBuffer.get());

		const mapped_vector<RenderPipelineSpecification, RenderPipeline>& pipelines = batcher.GetPipelines();
		const std::span<const DrawBatch<DRAWS_PER_BATCH>> drawBatches = batcher.GetDrawBatches();
		size_t batchCount = drawBatches.size();
		size_t offset = 0;

		batcher.MakeBatchTexturesResident();

		for (size_t i = 0; i < batchCount; i++)
		{
			const DrawBatch<DRAWS_PER_BATCH>& batch = drawBatches[i];
			const RenderPipeline& pipeline = pipelines[i];

			pipeline.Bind();

			const mapped_vector<AssetHandle, Ref<Material>>& batchMaterials = batch.GetMaterialInstances();
			for (size_t i = 0; i < batchMaterials.size(); i++)
			{
				batchMaterials[i]->BindlessUpload(i);
			}

			batch.UploadLocalMeshSubmissions(s_RenderData.MeshDataBuffer.get());
			RenderCommand::MultiDrawIndexedIndirect(batch.DrawCount(), offset);
			offset += batch.DrawCount();
			s_RenderData.Stats.DrawCalls++;
		}

		batcher.MakeBatchTexturesNonResident();

		s_RenderData.SceneDataBuffer = Renderer3DData::SceneData();
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;
		
		s_RenderData.Stats.PipelineCount += batchCount;
		batcher.Reset();
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

		meshHandle = (AssetManager::IsAssetHandleValid(meshHandle)) ? meshHandle : s_RenderData.SphereMeshHandle;;
		materialHandle = (AssetManager::IsAssetHandleValid(materialHandle)) ? materialHandle : s_RenderData.DefaultForwardMaterial;

		Ref<Mesh> meshAsset = AssetManager::GetAsset<Mesh>(meshHandle);

		MeshSubmissionData submissionData;
		submissionData.Transform = transform;
		submissionData.EntityID = entityID;
		submissionData.MaterialIndex = 0;

		DrawElementsIndirectCommand cmd;
		cmd.Count = meshAsset->NumIndices();
		cmd.InstanceCount = 1;
		cmd.FirstIndex = meshAsset->BaseIndicesIndex();
		cmd.BaseVertex = meshAsset->BaseVertexIndex();
		cmd.BaseInstance = 0;

		Ref<Material> materialAsset = AssetManager::GetAsset<Material>(materialHandle);
		s_RenderData.DrawBatcher.SubmitMesh(cmd, submissionData, { cullState, depthState, blendState }, materialAsset);

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

		Ref<RenderPipeline> pipeline = RenderPipeline::Create({ { cullState, depthState, blendState }, material->GetShaderHandle() });
		pipeline->Bind();
		material->Bind();
		material->BindlessUpload(0);

		std::unordered_set<uint64_t> deviceHandleSet = std::unordered_set<uint64_t>();
		material->AddBindlessTextureHandlesToSet(&deviceHandleSet);

		MakeTextureSetResident(deviceHandleSet);

		MeshSubmissionData submission;
		submission.Transform = transform;
		submission.EntityID = entityID;
		submission.MaterialIndex = 0;
		s_RenderData.MeshDataBuffer->SetData({ &submission, sizeof(MeshSubmissionData), 0 }, false);

		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(Renderer3DData::SceneDataBuffer));

		Mesh::GetMasterVAO()->Bind();
		RenderCommand::DrawIndexedBaseVertex(meshAsset->BaseVertexIndex(), meshAsset->BaseIndicesIndex(), meshAsset->NumIndices());

		s_RenderData.Stats.MeshCount++;
		s_RenderData.Stats.DrawCalls++;

		MakeTextureSetNonResident(deviceHandleSet);
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

	void Renderer::MakeTextureSetResident(const std::unordered_set<uint64_t>& textureSet)
	{
		PE_PROFILE_FUNCTION();
		for (auto it = textureSet.begin(); it != textureSet.end(); it++)
		{
			RenderCommand::MakeTextureResident(*it);
		}
	}

	void Renderer::MakeTextureSetNonResident(const std::unordered_set<uint64_t>& textureSet)
	{
		PE_PROFILE_FUNCTION();
		for (auto it = textureSet.begin(); it != textureSet.end(); it++)
		{
			RenderCommand::MakeTextureNonResident(*it);
		}
	}
}