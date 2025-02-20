#include <PaulEngine.h>
#include <imgui.h>
#include <glm/ext/matrix_transform.hpp>
#include <Platform/OpenGL/OpenGLShader.h>

#include <PaulEngine/OrthographicCameraController.h>

class TestLayer : public PaulEngine::Layer {
public:
	TestLayer() : Layer("Test Layer"), m_CameraController(1.6f / 0.9f, true, 1.0f, 1.5f) {
		m_VertexArray.reset(PaulEngine::VertexArray::Create());

		// Vertex buffer

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f,		0.8f, 0.3f, 0.2f, 1.0f,
			0.5, -0.5f, 0.0f,		0.2f, 0.8f, 0.3f, 1.0f,
			0.0f, 0.5f, 0.0f,		0.3f, 0.2f, 0.8f, 1.0f
		};

		PaulEngine::Ref<PaulEngine::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(PaulEngine::VertexBuffer::Create(vertices, sizeof(vertices)));

		PaulEngine::BufferLayout layout = {
			{ PaulEngine::ShaderDataType::Float3, "a_Position", false },
			{ PaulEngine::ShaderDataType::Float4, "a_Colour", true }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		// Index buffer
		uint32_t indices[3] = { 0, 1, 2 };
		PaulEngine::Ref<PaulEngine::IndexBuffer> indexBuffer;
		indexBuffer.reset(PaulEngine::IndexBuffer::Create(indices, 3));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// Square
		// ------
		float squareVertices[4 * 5] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			0.5, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f
		};

		m_SquareVertexArray.reset(PaulEngine::VertexArray::Create());
		PaulEngine::Ref<PaulEngine::VertexBuffer> squareVB;
		squareVB.reset(PaulEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ PaulEngine::ShaderDataType::Float3, "a_Position", false },
			{ PaulEngine::ShaderDataType::Float2, "a_TexCoords", true }
		});
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		uint32_t square_indices[6] = { 0, 1, 2, 2, 3, 0 };
		PaulEngine::Ref<PaulEngine::IndexBuffer> squareIB;
		squareIB.reset(PaulEngine::IndexBuffer::Create(square_indices, 6));
		m_SquareVertexArray->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core

			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec4 a_Colour;

			uniform mat4 u_ModelMatrix;
			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Colour;

			void main()
			{
				v_Colour = a_Colour;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout (location = 0) out vec4 colour;

			in vec3 v_Position;
			in vec4 v_Colour;			

			void main()
			{
				colour = v_Colour;
				//colour = vec4(v_Position * 0.5 + 0.5, 1.0);
			}
		)";

		std::string flatColourVertexSrc = R"(
			#version 330 core

			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec2 a_TexCoords;

			uniform mat4 u_ModelMatrix;
			uniform mat4 u_ViewProjection;

			void main()
			{
				gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColourFragmentSrc = R"(
			#version 330 core

			layout (location = 0) out vec4 colour;

			uniform vec4 u_Colour;

			void main()
			{
				//colour = v_Colour;
				//colour = vec4(v_Position * 0.5 + 0.5, 1.0);
				//colour = vec4(0.5, 0.5, 0.5, 1.0);
				colour = u_Colour;
			}
		)";

		// Shader
		m_ShaderLibrary.Add(PaulEngine::Shader::Create("TestShader", vertexSrc, fragmentSrc));
		m_ShaderLibrary.Add(PaulEngine::Shader::Create("FlatColourShader", flatColourVertexSrc, flatColourFragmentSrc));
		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");
		
		m_Texture = PaulEngine::Texture2D::Create("assets/textures/Checkerboard.png");
		std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);

		m_TransparentTexture = PaulEngine::Texture2D::Create("assets/textures/awesomeface.png");
	}

	void OnUpdate(const PaulEngine::Timestep timestep) override
	{
		m_CameraController.OnUpdate(timestep);

		PaulEngine::RenderCommand::SetViewport({ 0, 0 }, { PaulEngine::Application::Get().GetWindow().GetWidth(), PaulEngine::Application::Get().GetWindow().GetHeight()});

		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();

		PaulEngine::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::vec4 redColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColour = glm::vec4(0.2f, 0.3f, 0.8f, 1.0f);

		auto flatColourShader = m_ShaderLibrary.Get("FlatColourShader");
		std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(flatColourShader)->Bind();
		std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(flatColourShader)->UploadUniformFloat4("u_Colour", m_SquareColour);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				glm::vec3 pos = glm::vec3(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				PaulEngine::Renderer::Submit(flatColourShader, m_SquareVertexArray, transform);
			}
		}

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::scale(transform, glm::vec3(1.5f, 1.5f, 1.0f));
		//m_Texture->Bind(0);
		m_TransparentTexture->Bind(0);
		PaulEngine::Renderer::Submit(m_ShaderLibrary.Get("Texture"), m_SquareVertexArray, transform);

		//PaulEngine::Renderer::Submit(m_ShaderLibrary.Get("TestShader"), m_VertexArray);

		PaulEngine::Renderer::EndScene();
	}

	void OnImGuiRender() override {
		ImGui::Begin("Colour Edit");
		ImGui::ColorPicker4("Square Colour", &m_SquareColour[0], ImGuiColorEditFlags_AlphaPreviewHalf);
		ImGui::End();
	}

	void OnEvent(PaulEngine::Event& e) override {
		m_CameraController.OnEvent(e);
	}

private:
	PaulEngine::ShaderLibrary m_ShaderLibrary;

	PaulEngine::Ref<PaulEngine::VertexArray> m_VertexArray;
	PaulEngine::Ref<PaulEngine::VertexArray> m_SquareVertexArray;

	PaulEngine::Ref<PaulEngine::Texture2D> m_Texture, m_TransparentTexture;

	glm::vec4 m_SquareColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);

	PaulEngine::OrthographicCameraController m_CameraController;
};

class Sandbox : public PaulEngine::Application {
public:
	Sandbox() {
		PushLayer(new TestLayer());
	}
	~Sandbox() {}
};

PaulEngine::Application* PaulEngine::CreateApplication() {
	return new Sandbox();
}