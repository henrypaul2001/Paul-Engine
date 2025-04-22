#include "pepch.h"
#include "PaulEngine/Project/Project.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "UniformBuffer.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "Material.h"

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
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;

		AssetHandle DefaultLitShaderHandle;
		AssetHandle DefaultLitMaterialHandle;

		struct CameraData
		{
			glm::mat4 ViewProjection;
			glm::vec3 ViewPos;
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

		struct SceneData
		{
			Renderer::DirectionalLight DirLight;
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

		// -- Quad --
		// ----------
		{
			s_RenderData.QuadVertexArray = VertexArray::Create();
			s_RenderData.QuadVertexBuffer = VertexBuffer::Create(4 * sizeof(QuadVertex));
			s_RenderData.QuadVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position", false },
				{ ShaderDataType::Float3, "a_Normal", true },
				{ ShaderDataType::Float2, "a_TexCoords", true },
				{ ShaderDataType::Float3, "a_Tangent", true },
				{ ShaderDataType::Float3, "a_Bitangent", true }
			});
			s_RenderData.QuadVertexArray->AddVertexBuffer(s_RenderData.QuadVertexBuffer);

			QuadVertex vertices[4] = {
				{ {	-0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } }
			};
			s_RenderData.QuadVertexBuffer->SetData(&vertices[0], sizeof(QuadVertex) * 4);

			uint32_t quadIndices[6] = {
				0, 1, 2,
				2, 3, 0
			};

			ComputeTangentsIndexed(&vertices[0], &quadIndices[0], 6, 4);

			Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, 6);
			s_RenderData.QuadVertexArray->SetIndexBuffer(quadIB);
		}

		// -- Cube --
		// ----------
		{
			s_RenderData.CubeVertexArray = VertexArray::Create();
			s_RenderData.CubeVertexBuffer = VertexBuffer::Create(24 * sizeof(QuadVertex));
			s_RenderData.CubeVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position", false },
				{ ShaderDataType::Float3, "a_Normal", true },
				{ ShaderDataType::Float2, "a_TexCoords", true },
				{ ShaderDataType::Float3, "a_Tangent", true },
				{ ShaderDataType::Float3, "a_Bitangent", true }
			});
			s_RenderData.CubeVertexArray->AddVertexBuffer(s_RenderData.CubeVertexBuffer);

			QuadVertex vertices[24] = {
				{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f,  -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f,  0.5f,  -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -0.5f, 0.5f,  -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				
				{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f,  -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -0.5f, 0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
				
				{ { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
				
				{ { 0.5f, -0.5f, -0.5f  }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f,  0.5f, -0.5f  }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f,  0.5f,  0.5f  }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
				{ { 0.5f, -0.5f,  0.5f  }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f } },
				
				{ { -0.5f, -0.5f, -0.5f,}, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
				{ { 0.5f,  -0.5f, -0.5f, }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
				{ { 0.5f,  -0.5f,  0.5f, }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
				{ { -0.5f, -0.5f,  0.5f,}, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
				
				{ { 0.5f,  0.5f, -0.5f, }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
				{ { -0.5f, 0.5f, -0.5f,}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
				{ { -0.5f, 0.5f,  0.5f,}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
				{ { 0.5f,  0.5f,  0.5f, }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }
			};
			s_RenderData.CubeVertexBuffer->SetData(&vertices[0], sizeof(QuadVertex) * 24);

			uint32_t cubeIndices[36] = {
				// front and back
				0, 3, 2,
				2, 1, 0,
				4, 5, 6,
				6, 7 ,4,
				
				// left and right
				11, 8, 9,
				9, 10, 11,
				12, 13, 14,
				14, 15, 12,
				
				// bottom and top
				16, 17, 18,
				18, 19, 16,
				20, 21, 22,
				22, 23, 20
			};

			ComputeTangentsIndexed(&vertices[0], &cubeIndices[0], 36, 24);

			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, 36);
			s_RenderData.CubeVertexArray->SetIndexBuffer(cubeIB);
		}

		s_RenderData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraBuffer), 0);
		s_RenderData.MeshDataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::MeshDataBuffer), 1);
		s_RenderData.SceneDataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::SceneDataBuffer), 2);

		DirectionalLight dirLight;
		dirLight.Direction = glm::vec4(-0.2, -0.5, -0.3, 1.0f);
		dirLight.Ambient = glm::vec4(0.2, 0.2, 0.2, 1.0f);
		dirLight.Diffuse = glm::vec4(0.5, 0.5, 0.5, 1.0f);
		dirLight.Specular = glm::vec4(1.0, 1.0, 1.0, 1.0f);

		s_RenderData.SceneDataBuffer.DirLight = dirLight;
		s_RenderData.SceneDataUniformBuffer->SetData(&s_RenderData.SceneDataBuffer, sizeof(Renderer3DData::SceneDataBuffer));
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_RenderData.CameraBuffer.ViewPos = camera.GetPosition();
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));

		s_RenderData.SceneDataBuffer.DirLight = Renderer::DirectionalLight();
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RenderData.CameraBuffer.ViewPos = transform[3];
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));

		s_RenderData.SceneDataBuffer.DirLight = Renderer::DirectionalLight();
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
			const RenderPass& renderPass = pipeline->GetRenderPass();
			for (const DrawSubmission& d : renderPass.DrawList) {
				s_RenderData.MeshDataBuffer.Transform = d.Transform;
				s_RenderData.MeshDataBuffer.EntityID = d.EntityID;
				s_RenderData.MeshDataUniformBuffer->SetData(&s_RenderData.MeshDataBuffer, sizeof(Renderer3DData::MeshDataBuffer));

				RenderCommand::DrawIndexed(d.VertexArray, d.VertexArray->GetIndexBuffer()->GetCount());
				s_RenderData.Stats.DrawCalls++;
			}
		}

		s_RenderData.MeshDataBuffer = Renderer3DData::MeshSubmissionData();

		s_RenderData.PipelineKeyMap.clear();
	}

	void Renderer::SubmitDefaultCube(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID)
	{
		SubmitMesh(s_RenderData.CubeVertexArray, materialHandle, transform, depthState, cullState, entityID);
	}

	void Renderer::SubmitDefaultQuad(AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID)
	{
		SubmitMesh(s_RenderData.QuadVertexArray, materialHandle, transform, depthState, cullState, entityID);
	}

	void Renderer::SubmitMesh(Ref<VertexArray> vertexArray, AssetHandle materialHandle, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID)
	{
		PE_PROFILE_FUNCTION();

		DrawSubmission draw;
		draw.VertexArray = vertexArray;
		draw.MaterialHandle = (AssetManager::IsAssetHandleValid(materialHandle)) ? materialHandle : s_RenderData.DefaultLitMaterialHandle;
		draw.Transform = transform;
		draw.EntityID = entityID;

		// TODO: Possibility for automatic instancing if a duplicate pipeline state is found AND a duplicate vertex array

		// Check for duplicate pipeline state
		std::string pipelineKey = ConstructPipelineStateKey(draw.MaterialHandle, depthState, cullState);

		if (s_RenderData.PipelineKeyMap.find(pipelineKey) != s_RenderData.PipelineKeyMap.end())
		{
			// Duplicate pipeline state
			s_RenderData.PipelineKeyMap[pipelineKey]->GetRenderPass().DrawList.push_back(draw);
		}
		else
		{
			// Unique pipeline state
			s_RenderData.PipelineKeyMap[pipelineKey] = RenderPipeline::Create(cullState, depthState, draw.MaterialHandle);
			s_RenderData.PipelineKeyMap[pipelineKey]->GetRenderPass().DrawList.push_back(draw);
			s_RenderData.Stats.PipelineCount++;
		}

		s_RenderData.Stats.MeshCount++;
	}

	void Renderer::SubmitDirectionalLightSource(const DirectionalLight& light)
	{
		s_RenderData.SceneDataBuffer.DirLight = light;
	}

	void Renderer::DrawDefaultCubeImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID)
	{
		DrawMeshImmediate(s_RenderData.CubeVertexArray, material, transform, depthState, cullState, entityID);
	}

	void Renderer::DrawDefaultQuadImmediate(Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID)
	{
		DrawMeshImmediate(s_RenderData.QuadVertexArray, material, transform, depthState, cullState, entityID);
	}

	void Renderer::DrawMeshImmediate(Ref<VertexArray> vertexArray, Ref<Material> material, const glm::mat4& transform, DepthState depthState, FaceCulling cullState, int entityID)
	{
		PE_PROFILE_FUNCTION();

		s_RenderData.CameraUniformBuffer->Bind(0);
		s_RenderData.MeshDataUniformBuffer->Bind(1);

		Ref<RenderPipeline> pipeline = RenderPipeline::Create(cullState, depthState, 0);
		pipeline->Bind();
		material->Bind();

		s_RenderData.MeshDataBuffer.Transform = transform;
		s_RenderData.MeshDataBuffer.EntityID = entityID;
		s_RenderData.MeshDataUniformBuffer->SetData(&s_RenderData.MeshDataBuffer, sizeof(Renderer3DData::MeshDataBuffer));

		RenderCommand::DrawIndexed(vertexArray, vertexArray->GetIndexBuffer()->GetCount());

		s_RenderData.Stats.MeshCount++;
		s_RenderData.Stats.DrawCalls++;
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

		s_RenderData.DefaultLitShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/Renderer3D_DefaultLit.glsl", true);
		s_RenderData.DefaultLitMaterialHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "materials/DefaultLit.pmat", true);
	}

	std::string Renderer::ConstructPipelineStateKey(const AssetHandle material, const DepthState depthState, const FaceCulling cullState)
	{
		PE_PROFILE_FUNCTION();
		std::string materialString = std::to_string((uint64_t)material);
		std::string depthString = std::to_string((int)depthState.Func) + std::to_string((int)depthState.Test) + std::to_string((int)depthState.Write);
		std::string cullString = std::to_string((int)cullState);

		return materialString + depthString + cullString;
	}
}