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


		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> cubeModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/DefaultCube.fbx");
		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> sphereModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/DefaultSphere.fbx");
		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> gobletsModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/brass_goblets.obj");
	
		//BatchedMesh sphereMesh = m_MeshManager.RegisterLoadedMesh(sphereModel->ParsedMeshes[0].Vertices, sphereModel->ParsedMeshes[0].Indices);
		BatchedMesh cubeMesh = m_MeshManager.RegisterLoadedMesh(cubeModel->ParsedMeshes[0].Vertices, cubeModel->ParsedMeshes[0].Indices);
		BatchedMesh gobletMesh = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[0].Vertices, gobletsModel->ParsedMeshes[0].Indices);

		//m_MeshList.push_back(sphereMesh);
		m_MeshList.push_back(cubeMesh);
		m_MeshList.push_back(gobletMesh);

		glm::mat4 sphereTransform = glm::mat4(1.0f);
		sphereTransform = glm::translate(sphereTransform, glm::vec3(0.0f, 0.0f, -2.0f));
		m_MeshTransforms.push_back(sphereTransform);

		glm::mat4 cubeTransform = glm::mat4(1.0f);
		cubeTransform = glm::translate(cubeTransform, glm::vec3(-1.0f, 0.0f, -1.0f));
		m_MeshTransforms.push_back(cubeTransform);

		m_SceneCamera = PaulEngine::SceneCamera(PaulEngine::SceneCameraType::SCENE_CAMERA_PERSPECTIVE);
		m_CameraTransform = glm::mat4(1.0f);

		s_CameraUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_CameraBuffer), 0);
		s_MeshDataUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_MeshDataBuffer), 1);
		s_SceneDataUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_SceneDataBuffer), 2);

		s_CameraUniformBuffer->Bind(0);
		s_MeshDataUniformBuffer->Bind(1);
		s_SceneDataUniformBuffer->Bind(2);

		s_CameraBuffer.View = glm::inverse(m_CameraTransform);
		s_CameraBuffer.Projection = m_SceneCamera.GetProjection();
		s_CameraBuffer.ViewPos = m_CameraTransform[3];
		s_CameraBuffer.Gamma = m_SceneCamera.GetGamma();
		s_CameraBuffer.Exposure = m_SceneCamera.GetExposure();
		s_CameraUniformBuffer->SetData(&s_CameraBuffer, sizeof(s_CameraBuffer));

		s_SceneDataUniformBuffer->SetData(&s_SceneDataBuffer, sizeof(s_SceneDataBuffer));

		m_Shader = PaulEngine::Shader::Create("assets/shaders/Renderer3D_DefaultLit.glsl");
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
	}

	bool Sandbox::OnKeyUp(PaulEngine::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == PE_KEY_ESCAPE) { PaulEngine::Application::Get().Close(); return true; }
	}
}