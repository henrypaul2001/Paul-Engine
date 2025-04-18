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
		glm::vec2 TexCoords;
	};

	struct Renderer3DData
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;

		AssetHandle TestMaterialShaderHandle;
		AssetHandle TestMaterialHandle;

		struct CameraData
		{
			glm::mat4 ViewProjection;
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

		Renderer::Statistics Stats;

		std::unordered_map<std::string, Ref<RenderPipeline>> PipelineKeyMap;

		Ref<Texture2DArray> TestTextureArray;
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
				{ ShaderDataType::Float2, "a_TexCoords", true },
				});
			s_RenderData.QuadVertexArray->AddVertexBuffer(s_RenderData.QuadVertexBuffer);

			QuadVertex vertices[4] = {
				{ {	-0.5f, -0.5f, 0.0f	}, { 0.0f, 0.0f } },
				{ { 0.5f, -0.5f, 0.0f	}, { 1.0f, 0.0f } },
				{ { 0.5f, 0.5f, 0.0f	}, { 1.0f, 1.0f } },
				{ { -0.5f, 0.5f, 0.0f	}, { 0.0f, 1.0f } }
			};
			s_RenderData.QuadVertexBuffer->SetData(&vertices[0], sizeof(QuadVertex) * 4);

			uint32_t quadIndices[6] = {
				0, 1, 2,
				2, 3, 0
			};

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
				{ ShaderDataType::Float2, "a_TexCoords", true },
			});
			s_RenderData.CubeVertexArray->AddVertexBuffer(s_RenderData.CubeVertexBuffer);

			QuadVertex vertices[24] = {
				{ { -0.5f, -0.5f, -0.5f },  { 0.0f, 0.0f } },
				{ { 0.5f, -0.5f, -0.5f },   { 1.0f, 0.0f } },
				{ { 0.5f,  0.5f, -0.5f },   { 1.0f, 1.0f } },
				{ { -0.5f,  0.5f, -0.5f },  { 0.0f, 1.0f } },
				{ { -0.5f, -0.5f,  0.5f },  { 0.0f, 0.0f } },
				{ { 0.5f, -0.5f,  0.5f },   { 1.0f, 0.0f } },
				{ { 0.5f,  0.5f,	0.5f }, { 1.0f, 1.0f } },
				{ { -0.5f,  0.5f,  0.5f },  { 0.0f, 1.0f } },
													   
				{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
				{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f } },
				{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
				{ { 0.5f, -0.5f, -0.5f  }, { 0.0f, 0.0f } },
				{ { 0.5f,  0.5f, -0.5f  }, { 1.0f, 0.0f } },
				{ { 0.5f,  0.5f,  0.5f  }, { 1.0f, 1.0f } },
				{ { 0.5f, -0.5f,  0.5f  }, { 0.0f, 1.0f } },
													   
				{ { -0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f } },
				{ { 0.5f, -0.5f, -0.5f, }, { 1.0f, 0.0f } },
				{ { 0.5f, -0.5f,  0.5f, }, { 1.0f, 1.0f } },
				{ { -0.5f, -0.5f,  0.5f,}, { 0.0f, 1.0f } },
				{ { 0.5f,  0.5f, -0.5f, }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f,}, { 1.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f,}, { 1.0f, 1.0f } },
				{ { 0.5f,  0.5f,  0.5f, }, { 0.0f, 1.0f } }
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

			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, 36);
			s_RenderData.CubeVertexArray->SetIndexBuffer(cubeIB);
		}

		s_RenderData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraBuffer), 0);
		s_RenderData.MeshDataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::MeshDataBuffer), 1);
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		PE_PROFILE_FUNCTION();

		EndScene();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));
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
		draw.MaterialHandle = (AssetManager::IsAssetHandleValid(materialHandle)) ? materialHandle : s_RenderData.TestMaterialHandle;
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

		s_RenderData.TestMaterialShaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/MaterialTest.glsl", true);
		s_RenderData.TestMaterialHandle = assetManager->ImportAsset("materials/TestMaterial.pmat", true);

		AssetHandle shaderHandle = assetManager->ImportAsset(engineAssetsRelativeToProjectAssets / "shaders/TextureArrayTest.glsl", true);
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