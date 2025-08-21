#include "Sandbox.h"
#include <imgui.h>
#include <PaulEngine/Asset/MeshImporter.h>

#include <glad/gl.h>

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

	GLuint64 GetGLTextureHandle(PaulEngine::Ref<PaulEngine::Texture2D> texture)
	{
		const GLuint64 handle = glGetTextureHandleARB(texture->GetRendererID());
		if (handle == 0)
		{
			PE_ERROR("Error getting GL64 texture handle");
		}
		return handle;
	}

	void Sandbox::OnAttach()
	{
		PE_PROFILE_FUNCTION();

		m_Camera = PaulEngine::CreateRef<PaulEngine::EditorCamera>(PaulEngine::EditorCamera(90.0f, 1.778f, 0.01f, 1000.0f));

		// Load textures
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/rusted_iron/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/brick_wall/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/cobble_floor/diffuse.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/gold/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/leather/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/marble_tile/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/metal_grid/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/metal_vent/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/plastic/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/sci-fi_wall/albedo.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/space_blanket/albedo.png")); // 10

		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/rusted_iron/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/brick_wall/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/cobble_floor/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/gold/normal.png")); // 14
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/leather/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/marble_tile/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/metal_grid/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/metal_vent/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/plastic/normal.png")); // 19
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/sci-fi_wall/normal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/space_blanket/normal.png")); // 21

		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/rusted_iron/metallic.png")); // 22
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/brick_wall/specular.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/cobble_floor/specular.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/gold/metallic.png")); // 25
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/marble_tile/metal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/metal_grid/metal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/metal_vent/metal.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/plastic/roughness.png")); // 29
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/sci-fi_wall/metallic.png"));
		m_Textures.push_back(PaulEngine::TextureImporter::LoadTexture2D("assets/textures/space_blanket/metallic.png"));

		m_LocalTextureBuffer = std::vector<GLuint64>();
		m_LocalTextureBuffer.reserve(m_Textures.size());
		for (PaulEngine::Ref<PaulEngine::Texture2D> texture : m_Textures)
		{
			m_LocalTextureBuffer.push_back(GetGLTextureHandle(texture));
		}

		//glCreateBuffers(1, &m_TextureBufferID);
		//glNamedBufferStorage(m_TextureBufferID, sizeof(GLuint64) * m_LocalTextureBuffer.size(), m_LocalTextureBuffer.data(), GL_DYNAMIC_STORAGE_BIT);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_TextureBufferID);

		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> cubeModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/DefaultCube.fbx");
		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> sphereModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/DefaultSphere.fbx");
		PaulEngine::Ref<PaulEngine::ParsedModelLoadResult> gobletsModel = PaulEngine::MeshImporter::ParseModelFileRaw("assets/models/brass_goblets.obj");
	
		BatchedMesh sphereMesh = m_MeshManager.RegisterLoadedMesh(sphereModel->ParsedMeshes[0].Vertices, sphereModel->ParsedMeshes[0].Indices);
		BatchedMesh cubeMesh = m_MeshManager.RegisterLoadedMesh(cubeModel->ParsedMeshes[0].Vertices, cubeModel->ParsedMeshes[0].Indices);
		BatchedMesh gobletMesh0 = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[0].Vertices, gobletsModel->ParsedMeshes[0].Indices);
		BatchedMesh gobletMesh1 = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[1].Vertices, gobletsModel->ParsedMeshes[1].Indices);
		BatchedMesh gobletMesh2 = m_MeshManager.RegisterLoadedMesh(gobletsModel->ParsedMeshes[2].Vertices, gobletsModel->ParsedMeshes[2].Indices);

		SubmitMesh(sphereMesh);
		SubmitMesh(gobletMesh0);
		SubmitMesh(gobletMesh1);
		SubmitMesh(gobletMesh2);
		SubmitMesh(cubeMesh);

		glm::mat4 sphereTransform = glm::mat4(1.0f);
		sphereTransform = glm::translate(sphereTransform, glm::vec3(0.0f, 0.0f, -2.0f));
		m_MeshTransforms.push_back(sphereTransform);
		m_MaterialIDs.push_back(2);

		glm::mat4 gobletTransform = glm::mat4(1.0f);
		gobletTransform = glm::translate(gobletTransform, glm::vec3(1.0f, 0.0f, -1.0f));
		gobletTransform = glm::scale(gobletTransform, glm::vec3(2.0f));
		m_MeshTransforms.push_back(gobletTransform);
		m_MeshTransforms.push_back(gobletTransform);
		m_MeshTransforms.push_back(gobletTransform);

		m_MaterialIDs.push_back(1);
		m_MaterialIDs.push_back(1);
		m_MaterialIDs.push_back(1);

		glm::mat4 cubeTransform = glm::mat4(1.0f);
		cubeTransform = glm::translate(cubeTransform, glm::vec3(-1.0f, 0.0f, -1.0f));
		m_MeshTransforms.push_back(cubeTransform);
		m_MaterialIDs.push_back(0);


		// Create grid of cubes
		glm::vec3 origin = glm::vec3(5.0f, 0.0f, -1.0f);
		glm::vec3 delta = glm::vec3(5.0f);
		// 250,047 cubes
		const int xNum = 63;
		const int yNum = 63;
		const int zNum = 63;
		int materialID = 0;
		for (int x = 0; x < xNum; x++)
		{
			for (int y = 0; y < yNum; y++)
			{
				for (int z = 0; z < zNum; z++)
				{
					SubmitMesh(cubeMesh);
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), origin);
					transform = glm::translate(transform, delta * glm::vec3(x, y, z));
					transform = glm::scale(transform, glm::vec3(3.0f));
					m_MeshTransforms.push_back(transform);
					m_MaterialIDs.push_back(materialID++);
					if (materialID >= 6)
					{
						materialID = 0;
					}
				}
			}
		}

		s_CameraUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_CameraBuffer), 0);
		s_SceneDataUniformBuffer = PaulEngine::UniformBuffer::Create(sizeof(s_SceneDataBuffer), 2);

		s_CameraUniformBuffer->Bind(0);
		s_SceneDataUniformBuffer->Bind(2);

		s_SceneDataUniformBuffer->SetData(&s_SceneDataBuffer, sizeof(s_SceneDataBuffer));

		m_Shader = PaulEngine::Shader::Create("assets/shaders/Renderer3D_DefaultLit.glsl");

		m_ViewportWidth = 0;
		m_ViewportHeight = 0;

		// Create draw command buffer
		glCreateBuffers(1, &m_DrawCommandBufferID);
		glNamedBufferStorage(m_DrawCommandBufferID, sizeof(DrawElementsIndirectCommand) * MAX_DRAW_COMMANDS, nullptr, GL_DYNAMIC_STORAGE_BIT);
		m_DrawCommandBufferSize = 0;

		m_LocalCommandsBuffer = std::vector<DrawElementsIndirectCommand>(MAX_DRAW_COMMANDS);
	
		// Create mesh submission buffer
		glCreateBuffers(1, &m_MeshSubmissionBufferID);
		glNamedBufferStorage(m_MeshSubmissionBufferID, sizeof(MeshSubmissionData) * MAX_DRAW_COMMANDS, nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_MeshSubmissionBufferID);
		m_MeshSubmissionBufferSize = 0;

		m_LocalMeshSubmissionBuffer = std::vector<MeshSubmissionData>(MAX_DRAW_COMMANDS);

		// Create material buffer
		glCreateBuffers(1, &m_MaterialBufferID);
		glNamedBufferStorage(m_MaterialBufferID, sizeof(BasicMaterial) * MAX_MATERIALS, nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_MaterialBufferID);
		m_MaterialBufferSize = 0;

		m_LocalMaterialBuffer = std::vector<BasicMaterial>(MAX_MATERIALS);

		// Load materials
		m_MaterialBufferSize = 0;

		// Create defaul material
		BasicMaterial plasticMaterial;
		plasticMaterial.Albedo = glm::vec4(1.0f);
		plasticMaterial.Specular = glm::vec4(1.0f);
		plasticMaterial.EmissionColour = glm::vec3(1.0f);
		plasticMaterial.EmissionStrength = 0.0f;
		plasticMaterial.Shininess = 16.0f;
		plasticMaterial.TextureScale = glm::vec2(1.0f);
		plasticMaterial.AlbedoTextureARB = m_LocalTextureBuffer[8];
		plasticMaterial.NormalTextureARB = m_LocalTextureBuffer[19];
		plasticMaterial.SpecularTextureARB = m_LocalTextureBuffer[29];
		plasticMaterial.UseNormalMap = 1;

		BasicMaterial goldMaterial;
		goldMaterial.Albedo = glm::vec4(1.0f);
		goldMaterial.Specular = glm::vec4(1.0f, 0.84f, 0.0f, 1.0f);
		goldMaterial.EmissionColour = glm::vec3(1.0f);
		goldMaterial.EmissionStrength = 0.0f;
		goldMaterial.Shininess = 32.0f;
		goldMaterial.TextureScale = glm::vec2(1.0f);
		goldMaterial.AlbedoTextureARB = m_LocalTextureBuffer[3];
		goldMaterial.NormalTextureARB = m_LocalTextureBuffer[14];
		goldMaterial.SpecularTextureARB = m_LocalTextureBuffer[25];
		goldMaterial.UseNormalMap = 1;

		BasicMaterial rustMaterial = goldMaterial;
		rustMaterial.Albedo = glm::vec4(1.0f);
		rustMaterial.Specular = rustMaterial.Albedo;
		rustMaterial.Shininess = 8.0f;
		rustMaterial.AlbedoTextureARB = m_LocalTextureBuffer[0];
		rustMaterial.NormalTextureARB = m_LocalTextureBuffer[11];
		rustMaterial.SpecularTextureARB = m_LocalTextureBuffer[22];
		rustMaterial.UseNormalMap = 1;

		BasicMaterial metalVentMaterial = goldMaterial;
		metalVentMaterial.Albedo = glm::vec4(1.0f);
		metalVentMaterial.Specular = glm::vec4(1.0f);
		metalVentMaterial.Shininess = 8.0f;
		metalVentMaterial.AlbedoTextureARB = m_LocalTextureBuffer[7];
		metalVentMaterial.NormalTextureARB = m_LocalTextureBuffer[18];
		metalVentMaterial.SpecularTextureARB = m_LocalTextureBuffer[28];
		metalVentMaterial.UseNormalMap = 1;

		BasicMaterial scifiMaterial = metalVentMaterial;
		scifiMaterial.Shininess = 16.0f;
		scifiMaterial.AlbedoTextureARB = m_LocalTextureBuffer[9];
		scifiMaterial.NormalTextureARB = m_LocalTextureBuffer[20];
		scifiMaterial.SpecularTextureARB = m_LocalTextureBuffer[30];
		scifiMaterial.UseNormalMap = 1;

		BasicMaterial metalGrid = metalVentMaterial;
		metalGrid.AlbedoTextureARB = m_LocalTextureBuffer[6];
		metalGrid.NormalTextureARB = m_LocalTextureBuffer[18];
		metalGrid.SpecularTextureARB = m_LocalTextureBuffer[27];
		metalGrid.UseNormalMap = 1;

		m_LocalMaterialBuffer[m_MaterialBufferSize++] = plasticMaterial;
		m_LocalMaterialBuffer[m_MaterialBufferSize++] = goldMaterial;
		m_LocalMaterialBuffer[m_MaterialBufferSize++] = rustMaterial;
		m_LocalMaterialBuffer[m_MaterialBufferSize++] = metalVentMaterial;
		m_LocalMaterialBuffer[m_MaterialBufferSize++] = scifiMaterial;
		m_LocalMaterialBuffer[m_MaterialBufferSize++] = metalGrid;

		glNamedBufferSubData(m_MaterialBufferID, 0, sizeof(BasicMaterial) * m_MaterialBufferSize, m_LocalMaterialBuffer.data());

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		PE_TRACE("Vertex buffer size: {0}", m_MeshManager.GetVertexBufferSize());
		PE_TRACE("Index buffer size : {0}", m_MeshManager.GetIndexBufferSize());
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

		// Set up camera buffer
		s_CameraBuffer.View = m_Camera->GetViewMatrix();
		s_CameraBuffer.Projection = m_Camera->GetProjection();
		s_CameraBuffer.ViewPos = m_Camera->GetPosition();
		s_CameraBuffer.Gamma = m_Camera->GetGamma();
		s_CameraBuffer.Exposure = m_Camera->GetExposure();
		s_CameraUniformBuffer->SetData(&s_CameraBuffer, sizeof(s_CameraBuffer));

		// Clear
		PaulEngine::RenderCommand::SetViewport({ 0, 0 }, { m_ViewportWidth, m_ViewportHeight });
		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();

		// Set up scene data
		PaulEngine::Renderer::DirectionalLight dirLight;
		dirLight.Ambient = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f);
		dirLight.Diffuse = glm::vec4(1.0f);
		dirLight.Specular = glm::vec4(1.0f);
		s_SceneDataBuffer.DirLights[0] = dirLight;
		s_SceneDataBuffer.ActiveDirLights = 1;
		s_SceneDataUniformBuffer->SetData(&s_SceneDataBuffer, sizeof(s_SceneDataBuffer));

		// Bind for drawing
		m_MeshManager.GetVertexArray()->Bind();
		m_Shader->Bind();

		// Init submissions
		m_MeshSubmissionBufferSize = 0;
		m_DrawCommandBufferSize = 0;

		m_TextureSubmissions.clear();

		// Submit meshes
		for (int i = 0; i < m_MeshTransforms.size(); i++)
		{
			MeshSubmissionData meshSubmission;
			meshSubmission.Transform = m_MeshTransforms[i];
			meshSubmission.EntityID = -1;
			meshSubmission.MaterialID = m_MaterialIDs[i];

			m_LocalMeshSubmissionBuffer[m_MeshSubmissionBufferSize++] = meshSubmission;

			const BasicMaterial& materialRef = m_LocalMaterialBuffer[meshSubmission.MaterialID];
			m_TextureSubmissions.insert(materialRef.AlbedoTextureARB);
			m_TextureSubmissions.insert(materialRef.NormalTextureARB);
			m_TextureSubmissions.insert(materialRef.SpecularTextureARB);
		}

		// Submit commands
		for (int i = 0; i < m_UniqueMeshList.size(); i++)
		{
			BatchedMesh& m = m_UniqueMeshList[i];

			DrawElementsIndirectCommand command;
			command.count = m.NumIndices;
			command.instanceCount = m_MeshInstances[m];
			command.firstIndex = m.BaseIndicesIndex;
			command.baseVertex = m.BaseVertexIndex;
			command.baseInstance = 0;

			m_LocalCommandsBuffer[m_DrawCommandBufferSize++] = command;
		}

		// Texture
		auto it = m_TextureSubmissions.cbegin();
		while (it != m_TextureSubmissions.cend())
		{
			glMakeTextureHandleResidentARB(*it);
			it++;
		}

		// Buffer draw commands
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawCommandBufferID);
		glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(DrawElementsIndirectCommand) * m_DrawCommandBufferSize, m_LocalCommandsBuffer.data());

		// Buffer per mesh data
		glNamedBufferSubData(m_MeshSubmissionBufferID, 0, sizeof(MeshSubmissionData) * m_MeshSubmissionBufferSize, m_LocalMeshSubmissionBuffer.data());

		// Execute draw commands in a single draw call
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, m_DrawCommandBufferSize, 0);

		it = m_TextureSubmissions.cbegin();
		while (it != m_TextureSubmissions.cend())
		{
			glMakeTextureHandleNonResidentARB(*it);
			it++;
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

	void Sandbox::SubmitMesh(BatchedMesh& mesh)
	{
		auto it = m_MeshInstances.find(mesh);
		if (it == m_MeshInstances.end())
		{
			m_MeshInstances[mesh] = 1;
			m_UniqueMeshList.push_back(mesh);
		}
		else
		{
			it->second += 1;
		}
	}
}