#include <PaulEngine.h>
#include <imgui.h>
#include <glm/ext/matrix_transform.hpp>
#include <Platform/OpenGL/OpenGLShader.h>

class TestLayer : public PaulEngine::Layer {
public:
	TestLayer() : Layer("Test Layer"), m_OrthoCamera(-1.6f, 1.6f, -0.9f, 0.9f) {
		m_VertexArray.reset(PaulEngine::VertexArray::Create());

		// Vertex buffer

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f,		0.8f, 0.3f, 0.2f, 1.0f,
			0.5, -0.5f, 0.0f,		0.2f, 0.8f, 0.3f, 1.0f,
			0.0f, 0.5f, 0.0f,		0.3f, 0.2f, 0.8f, 1.0f
		};

		std::shared_ptr<PaulEngine::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(PaulEngine::VertexBuffer::Create(vertices, sizeof(vertices)));

		PaulEngine::BufferLayout layout = {
			{ PaulEngine::ShaderDataType::Float3, "a_Position", false },
			{ PaulEngine::ShaderDataType::Float4, "a_Colour", true }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		// Index buffer
		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<PaulEngine::IndexBuffer> indexBuffer;
		indexBuffer.reset(PaulEngine::IndexBuffer::Create(indices, 3));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		// Square
		// ------
		float squareVertices[4 * 3] = {
			-0.5f, -0.5f, 0.0f,
			0.5, -0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f
		};

		m_SquareVertexArray.reset(PaulEngine::VertexArray::Create());
		std::shared_ptr<PaulEngine::VertexBuffer> squareVB;
		squareVB.reset(PaulEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ PaulEngine::ShaderDataType::Float3, "a_Position", false }
			});
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		uint32_t square_indices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<PaulEngine::IndexBuffer> squareIB;
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

			uniform mat4 u_ModelMatrix;
			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColourFragmentSrc = R"(
			#version 330 core

			layout (location = 0) out vec4 colour;

			uniform vec4 u_Colour;

			in vec3 v_Position;	

			void main()
			{
				//colour = v_Colour;
				//colour = vec4(v_Position * 0.5 + 0.5, 1.0);
				//colour = vec4(0.5, 0.5, 0.5, 1.0);
				colour = u_Colour;
			}
		)";

		// Shader
		m_Shader.reset(PaulEngine::Shader::Create(vertexSrc, fragmentSrc));
		m_FlatColourShader.reset(PaulEngine::Shader::Create(flatColourVertexSrc, flatColourFragmentSrc));

		//m_OrthoCamera.SetPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
	}

	void OnUpdate(const PaulEngine::Timestep timestep) override
	{
		const glm::vec3& currentPosition = m_OrthoCamera.GetPosition();
		const float currentRotation = m_OrthoCamera.GetRotation();
		float moveSpeed = 1.0f * timestep;
		float rotateSpeed = 1.5f * timestep;
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_UP)) {
			m_OrthoCamera.SetPosition(currentPosition + glm::vec3(0.0f, moveSpeed, 0.0f));
		}
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_DOWN)) {
			m_OrthoCamera.SetPosition(currentPosition + glm::vec3(0.0f, -moveSpeed, 0.0f));
		}
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_LEFT)) {
			m_OrthoCamera.SetPosition(currentPosition + glm::vec3(-moveSpeed, 0.0f, 0.0f));
		}
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_RIGHT)) {
			m_OrthoCamera.SetPosition(currentPosition + glm::vec3(moveSpeed, 0.0f, 0.0f));
		}
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_COMMA)) {
			m_OrthoCamera.SetRotation(currentRotation + 1.0f);
		}
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_PERIOD)) {
			m_OrthoCamera.SetRotation(currentRotation + -1.0f);
		}

		PaulEngine::RenderCommand::SetViewport({ 0, 0 }, { PaulEngine::Application::Get().GetWindow().GetWidth(), PaulEngine::Application::Get().GetWindow().GetHeight()});

		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();

		PaulEngine::Renderer::BeginScene(m_OrthoCamera);

		glm::vec4 redColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColour = glm::vec4(0.2f, 0.3f, 0.8f, 1.0f);

		std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(m_FlatColourShader)->Bind();
		std::dynamic_pointer_cast<PaulEngine::OpenGLShader>(m_FlatColourShader)->UploadUniformFloat4("u_Colour", m_SquareColour);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				glm::vec3 pos = glm::vec3(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				PaulEngine::Renderer::Submit(m_FlatColourShader, m_SquareVertexArray, transform);
			}
		}

		//PaulEngine::Renderer::Submit(m_FlatColourShader, m_SquareVertexArray);

		PaulEngine::Renderer::Submit(m_Shader, m_VertexArray);

		PaulEngine::Renderer::EndScene();
	}

	void OnImGuiRender() override {
		ImGui::Begin("Colour Edit");
		ImGui::ColorPicker4("Square Colour", &m_SquareColour[0], ImGuiColorEditFlags_AlphaPreviewHalf);
		ImGui::End();
	}

	void OnEvent(PaulEngine::Event& e) override {

	}

private:
	std::shared_ptr<PaulEngine::Shader> m_Shader;
	std::shared_ptr<PaulEngine::Shader> m_FlatColourShader;
	std::shared_ptr<PaulEngine::VertexArray> m_VertexArray;
	std::shared_ptr<PaulEngine::VertexArray> m_SquareVertexArray;

	glm::vec4 m_SquareColour = glm::vec4(0.8f, 0.2f, 0.3f, 1.0f);

	PaulEngine::OrthographicCamera m_OrthoCamera;
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