#include "pepch.h"
#include "PaulEngine/Project/Project.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Resource/UniformBuffer.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "Asset/Material.h"

#include "PaulEngine/Asset/TextureImporter.h"

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

		AssetHandle DefaultMaterial;

		struct CameraData
		{
			glm::mat4 ViewProjection;
			glm::vec3 ViewPos;
			float Gamma = 2.2f;
			float Exposure = 1.0f;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		struct MeshSubmissionData
		{
			glm::mat4 Transform;
			int EntityID;
		};
		MeshSubmissionData MeshDataBuffer;
		Ref<UniformBuffer> MeshDataUniformBuffer;

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

		std::unordered_map<std::string, Ref<RenderPipeline>> PipelineKeyMap;
	};
	static Renderer3DData s_RenderData;

	void Renderer::Init()
	{
		PE_PROFILE_FUNCTION();
		RenderCommand::Init();
		Renderer2D::Init();

		s_RenderData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraBuffer), 0);
		s_RenderData.MeshDataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::MeshDataBuffer), 1);
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

		s_RenderData.CameraBuffer.ViewProjection = camera.GetViewProjection();
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

		s_RenderData.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(worldTransform);
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

		s_RenderData.CameraBuffer.ViewProjection = projection * glm::inverse(worldTransform);
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
		s_RenderData.MeshDataUniformBuffer->Bind(1);
		s_RenderData.SceneDataUniformBuffer->Bind(2);
		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(s_RenderData.SceneDataBuffer));

		for (auto& [key, pipeline] : s_RenderData.PipelineKeyMap) {

			pipeline->Bind();
			const std::vector<DrawSubmission>& drawList = pipeline->GetDrawList();
			for (const DrawSubmission& d : drawList) {
				AssetHandle meshHandle = d.MeshHandle;
				if (AssetManager::IsAssetHandleValid(meshHandle))
				{
					Ref<VertexArray> vertexArray = AssetManager::GetAsset<Mesh>(meshHandle)->GetVertexArray();
					s_RenderData.MeshDataBuffer.Transform = d.Transform;
					s_RenderData.MeshDataBuffer.EntityID = d.EntityID;
					s_RenderData.MeshDataUniformBuffer->SetData(&s_RenderData.MeshDataBuffer, sizeof(Renderer3DData::MeshDataBuffer));

					RenderCommand::DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount());
					s_RenderData.Stats.DrawCalls++;
				}
				else
				{
					PE_CORE_WARN("Invalid mesh handle '{0}'", (uint64_t)meshHandle);
				}
			}
		}

		s_RenderData.MeshDataBuffer = Renderer3DData::MeshSubmissionData();

		s_RenderData.SceneDataBuffer = Renderer3DData::SceneData();
		s_RenderData.SceneBufferMetaData.DirLightsHead = 0;
		s_RenderData.SceneBufferMetaData.PointLightsHead = 0;
		s_RenderData.SceneBufferMetaData.SpotLightsHead = 0;

		s_RenderData.PipelineKeyMap.clear();
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
		draw.MaterialHandle = (AssetManager::IsAssetHandleValid(materialHandle)) ? materialHandle : s_RenderData.DefaultMaterial;
		draw.Transform = transform;
		draw.EntityID = entityID;

		// TODO: Possibility for automatic instancing if a duplicate pipeline state is found AND a duplicate mesh

		// Check for duplicate pipeline state
		std::string pipelineKey = ConstructPipelineStateKey(draw.MaterialHandle, depthState, cullState, blendState);

		if (s_RenderData.PipelineKeyMap.find(pipelineKey) != s_RenderData.PipelineKeyMap.end())
		{
			// Duplicate pipeline state
			s_RenderData.PipelineKeyMap[pipelineKey]->GetDrawList().push_back(draw);
		}
		else
		{
			// Unique pipeline state
			s_RenderData.PipelineKeyMap[pipelineKey] = RenderPipeline::Create(cullState, depthState, blendState, draw.MaterialHandle);
			s_RenderData.PipelineKeyMap[pipelineKey]->GetDrawList().push_back(draw);
			s_RenderData.Stats.PipelineCount++;
		}

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
		DrawMeshImmediate(cubeMesh->GetVertexArray(), material, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::DrawDefaultQuadImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		Ref<Mesh> quadMesh = AssetManager::GetAsset<Mesh>(s_RenderData.QuadMeshHandle);
		DrawMeshImmediate(quadMesh->GetVertexArray(), material, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::DrawDefaultSphereImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		Ref<Mesh> sphereMesh = AssetManager::GetAsset<Mesh>(s_RenderData.SphereMeshHandle);
		DrawMeshImmediate(sphereMesh->GetVertexArray(), material, transform, depthState, cullState, blendState, entityID);
	}

	void Renderer::DrawMeshImmediate(Ref<VertexArray> vertexArray, Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, BlendState blendState, int entityID)
	{
		PE_PROFILE_FUNCTION();

		s_RenderData.CameraUniformBuffer->Bind(0);
		s_RenderData.MeshDataUniformBuffer->Bind(1);
		s_RenderData.SceneDataUniformBuffer->Bind(2);

		Ref<RenderPipeline> pipeline = RenderPipeline::Create(cullState, depthState, blendState, 0);
		pipeline->Bind();
		material->Bind();

		s_RenderData.MeshDataBuffer.Transform = transform;
		s_RenderData.MeshDataBuffer.EntityID = entityID;
		s_RenderData.MeshDataUniformBuffer->SetData(&s_RenderData.MeshDataBuffer, sizeof(Renderer3DData::MeshDataBuffer));

		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(Renderer3DData::SceneDataBuffer));

		RenderCommand::DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount());

		s_RenderData.Stats.MeshCount++;
		s_RenderData.Stats.DrawCalls++;
	}

	bool Renderer::SetDefaultMaterial(AssetHandle materialHandle)
	{
		if (AssetManager::IsAssetHandleValid(materialHandle))
		{
			s_RenderData.DefaultMaterial = materialHandle;
			return true;
		}
		PE_CORE_ERROR("Invalid material handle '{0}'", (uint64_t)materialHandle);
		return false;
	}

	AssetHandle Renderer::DefaultLitShader()
	{
		return s_RenderData.DefaultLitShaderHandle;
	}

	AssetHandle Renderer::DefaultLitPBRShader()
	{
		return s_RenderData.DefaultLitPBRShaderHandle;
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

		if (!SetDefaultMaterial(Project::GetActive()->GetSpecification().DefaultMaterial))
		{
			SetDefaultMaterial(s_RenderData.DefaultLitMaterialHandle);
		}
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