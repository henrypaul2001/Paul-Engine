#include "pepch.h"
#include "PaulEngine/Project/Project.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "UniformBuffer.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "Material.h"

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

		Renderer::RenderPass CurrentRenderPass;

		Renderer::Statistics Stats;
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

		s_RenderData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraBuffer), 0);
		s_RenderData.MeshDataUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::MeshDataBuffer), 1);
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		PE_PROFILE_FUNCTION();
		s_RenderData.CurrentRenderPass = Renderer::RenderPass();

		s_RenderData.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_RenderData.CameraUniformBuffer->SetData(&s_RenderData.CameraBuffer, sizeof(Renderer3DData::CameraBuffer));
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		PE_PROFILE_FUNCTION();
		s_RenderData.CurrentRenderPass = Renderer::RenderPass();

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

		for (const DrawSubmission& s : s_RenderData.CurrentRenderPass.DrawList) {
			if (AssetManager::IsAssetHandleValid(s.MaterialHandle)) {
				AssetManager::GetAsset<Material>(s.MaterialHandle)->Bind();
			}
			else {
				AssetManager::GetAsset<Material>(s_RenderData.TestMaterialHandle)->Bind();
			}

			s_RenderData.MeshDataBuffer.Transform = s.Transform;
			s_RenderData.MeshDataBuffer.EntityID = s.EntityID;
			s_RenderData.MeshDataUniformBuffer->SetData(&s_RenderData.MeshDataBuffer, sizeof(Renderer3DData::MeshDataBuffer));

			RenderCommand::DrawIndexed(s.VertexArray, s.VertexArray->GetIndexBuffer()->GetCount());
			s_RenderData.Stats.DrawCalls++;
		}

		s_RenderData.CurrentRenderPass = Renderer::RenderPass();
		s_RenderData.MeshDataBuffer = Renderer3DData::MeshSubmissionData();
	}

	void Renderer::SubmitDefaultQuad(AssetHandle materialHandle, const glm::mat4& transform, int entityID)
	{
		PE_PROFILE_FUNCTION();

		DrawSubmission draw;
		draw.VertexArray = s_RenderData.QuadVertexArray;
		draw.MaterialHandle = materialHandle;
		draw.Transform = transform;
		draw.EntityID = entityID;
		s_RenderData.CurrentRenderPass.DrawList.push_back(draw);

		s_RenderData.Stats.MeshCount++;
	}

	void Renderer::SubmitMesh(Ref<VertexArray> vertexArray, AssetHandle materialHandle, const glm::mat4& transform, int entityID)
	{
		PE_PROFILE_FUNCTION();

		DrawSubmission draw;
		draw.VertexArray = vertexArray;
		draw.MaterialHandle = materialHandle;
		draw.Transform = transform;
		draw.EntityID = entityID;
		s_RenderData.CurrentRenderPass.DrawList.push_back(draw);

		s_RenderData.Stats.MeshCount++;
	}

	void Renderer::ResetStats()
	{
		s_RenderData.Stats.DrawCalls = 0;
		s_RenderData.Stats.MeshCount = 0;
		s_RenderData.Stats.MaterialCount = 0;
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
	}
}