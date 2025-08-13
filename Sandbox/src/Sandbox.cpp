#include "Sandbox.h"
#include <imgui.h>
#include <PaulEngine/Asset/MeshImporter.h>

#include <glad/glad.h>

namespace Sandbox
{
	Sandbox::Sandbox() : Layer("Sandbox_Layer") {}

	Sandbox::~Sandbox() {}

	struct CameraData
	{
		glm::mat4 View;
		glm::mat4 Projection;
		glm::vec3 ViewPos;
		float Gamma = 2.2f;
		float Exposure = 1.0f;
	};
	static CameraData s_CameraBuffer;
	static PaulEngine::Ref<PaulEngine::UniformBuffer> s_CameraUniformBuffer = nullptr;

	struct MeshSubmissionData
	{
		glm::mat4 Transform;
		int EntityID;
	};
	static MeshSubmissionData s_MeshDataBuffer;
	static PaulEngine::Ref<PaulEngine::UniformBuffer> s_MeshDataUniformBuffer = nullptr;

	struct SceneData
	{
		PaulEngine::Renderer::DirectionalLight DirLights[PaulEngine::Renderer::MAX_ACTIVE_DIR_LIGHTS];
		PaulEngine::Renderer::PointLight PointLights[PaulEngine::Renderer::MAX_ACTIVE_POINT_LIGHTS];
		PaulEngine::Renderer::SpotLight SpotLights[PaulEngine::Renderer::MAX_ACTIVE_SPOT_LIGHTS];
		int ActiveDirLights = 0;
		int ActivePointLights = 0;
		int ActiveSpotLights = 0;
	};
	static SceneData s_SceneDataBuffer;
	static PaulEngine::Ref<PaulEngine::UniformBuffer> s_SceneDataUniformBuffer = nullptr;

	void Sandbox::OnAttach()
	{
		PE_PROFILE_FUNCTION();

		m_Camera = PaulEngine::CreateRef<PaulEngine::EditorCamera>(PaulEngine::EditorCamera(90.0f, 1.778f, 0.01f, 1000.0f));

		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> cubeModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/DefaultCube.fbx");
		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> sphereModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/DefaultSphere.fbx");
		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> gobletsModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/brass_goblets.obj");
	
		BatchedMesh sphereMesh = m_MeshManager.RegisterLoadedMesh(sphereModel->ParsedMeshes[0].Vertices, sphereModel->ParsedMeshes[0].Indices);
		BatchedMesh cubeMesh = m_MeshManager.RegisterLoadedMesh(cubeModel->ParsedMeshes[0].Vertices, cubeModel->ParsedMeshes[0].Indices);
		BatchedMesh gobletMesh0 = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[0].Vertices, gobletsModel->ParsedMeshes[0].Indices);
		BatchedMesh gobletMesh1 = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[1].Vertices, gobletsModel->ParsedMeshes[1].Indices);
		BatchedMesh gobletMesh2 = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[2].Vertices, gobletsModel->ParsedMeshes[2].Indices);

		m_MeshList.push_back(sphereMesh);
		m_MeshList.push_back(cubeMesh);
		m_MeshList.push_back(gobletMesh0);
		m_MeshList.push_back(gobletMesh1);
		m_MeshList.push_back(gobletMesh2);

		glm::mat4 sphereTransform = glm::mat4(1.0f);
		sphereTransform = glm::translate(sphereTransform, glm::vec3(0.0f, 0.0f, -2.0f));
		m_MeshTransforms.push_back(sphereTransform);

		glm::mat4 cubeTransform = glm::mat4(1.0f);
		cubeTransform = glm::translate(cubeTransform, glm::vec3(-1.0f, 0.0f, -1.0f));
		m_MeshTransforms.push_back(cubeTransform);

		glm::mat4 gobletTransform = glm::mat4(1.0f);
		gobletTransform = glm::translate(gobletTransform, glm::vec3(1.0f, 0.0f, -1.0f));
		gobletTransform = glm::scale(gobletTransform, glm::vec3(2.0f));
		m_MeshTransforms.push_back(gobletTransform);
		m_MeshTransforms.push_back(gobletTransform);
		m_MeshTransforms.push_back(gobletTransform);

		s_CameraUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_CameraBuffer), 0);
		s_MeshDataUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_MeshDataBuffer), 1);
		s_SceneDataUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_SceneDataBuffer), 2);

		s_CameraUniformBuffer->Bind(0);
		s_MeshDataUniformBuffer->Bind(1);
		s_SceneDataUniformBuffer->Bind(2);

		s_SceneDataUniformBuffer->SetData(&s_SceneDataBuffer, sizeof(s_SceneDataBuffer));

		m_Shader = PaulEngine::Shader::Create("assets/shaders/Renderer3D_DefaultLit.glsl");

		m_ViewportWidth = 0;
		m_ViewportHeight = 0;
	}

	void Sandbox::OnDetach()
	{
		PE_PROFILE_FUNCTION();
	}

	static PaulEngine::Timestep deltaTime = 0.0f;
	void Sandbox::OnUpdate(const PaulEngine::Timestep timestep)
	{
		PE_PROFILE_FUNCTION();
		deltaTime = timestep;

		uint32_t newWidth = PaulEngine::Application::Get().GetWindow().GetWidth();
		uint32_t newHeight = PaulEngine::Application::Get().GetWindow().GetHeight();
		if (newWidth != m_ViewportWidth || newHeight != m_ViewportHeight)
		{
			PaulEngine::MainViewportResizeEvent e = PaulEngine::MainViewportResizeEvent(newWidth, newHeight);
			OnEvent(e);
		}

		m_Camera->OnUpdate(timestep, false);

		s_CameraBuffer.View = m_Camera->GetViewMatrix();
		s_CameraBuffer.Projection = m_Camera->GetProjection();
		s_CameraBuffer.ViewPos = m_Camera->GetPosition();
		s_CameraBuffer.Gamma = m_Camera->GetGamma();
		s_CameraBuffer.Exposure = m_Camera->GetExposure();
		s_CameraUniformBuffer->SetData(&s_CameraBuffer, sizeof(s_CameraBuffer));

		PaulEngine::RenderCommand::SetViewport({ 0, 0 }, { m_ViewportWidth, m_ViewportHeight });
		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();

		//SceneDataUniformBuffer->SetData(&SceneDataBuffer, sizeof(SceneDataBuffer));

		m_MeshManager.GetVertexArray()->Bind();
		m_Shader->Bind();

		for (int i = 0; i < m_MeshList.size(); i++)
		{
			BatchedMesh m = m_MeshList[i];

			s_MeshDataBuffer.Transform = m_MeshTransforms[i];
			s_MeshDataBuffer.EntityID = -1;
			s_MeshDataUniformBuffer->SetData(&s_MeshDataBuffer, sizeof(s_MeshDataBuffer));

			// Draw call
			glDrawElementsBaseVertex(GL_TRIANGLES, m.NumIndices, GL_UNSIGNED_INT, (void*)(m.BaseIndicesIndex * sizeof(uint32_t)), m.BaseVertexIndex);
		}
	}

	void Sandbox::OnImGuiRender()
	{
		PE_PROFILE_FUNCTION();
	}

	void Sandbox::OnEvent(PaulEngine::Event& e)
	{
		PE_PROFILE_FUNCTION();

		PaulEngine::EventDispatcher dispatcher = PaulEngine::EventDispatcher(e);
		dispatcher.DispatchEvent<PaulEngine::KeyReleasedEvent>(PE_BIND_EVENT_FN(Sandbox::OnKeyUp));
		dispatcher.DispatchEvent<PaulEngine::MainViewportResizeEvent>(PE_BIND_EVENT_FN(Sandbox::OnViewportResize));

		m_Camera->OnEvent(e);
	}

	bool Sandbox::OnKeyUp(PaulEngine::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == PE_KEY_ESCAPE) { PaulEngine::Application::Get().Close(); return true; }
	}

	bool Sandbox::OnViewportResize(PaulEngine::MainViewportResizeEvent& e)
	{
		m_ViewportWidth = e.GetWidth();
		m_ViewportHeight = e.GetHeight();
		m_Camera->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		return false;
	}
}