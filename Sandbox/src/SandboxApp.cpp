#include <PaulEngine.h>
#include <imgui.h>

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

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Colour;

			void main()
			{
				v_Colour = a_Colour;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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

		std::string vertexSrc2 = R"(
			#version 330 core

			layout (location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc2 = R"(
			#version 330 core

			layout (location = 0) out vec4 colour;

			in vec3 v_Position;	

			void main()
			{
				//colour = v_Colour;
				//colour = vec4(v_Position * 0.5 + 0.5, 1.0);
				colour = vec4(0.5, 0.5, 0.5, 1.0);
			}
		)";

		// Shader
		m_Shader.reset(new PaulEngine::Shader(vertexSrc, fragmentSrc));
		m_Shader2.reset(new PaulEngine::Shader(vertexSrc2, fragmentSrc2));

		//m_OrthoCamera.SetPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
	}

	void OnUpdate() override
	{
		const glm::vec3& currentPosition = m_OrthoCamera.GetPosition();
		const float currentRotation = m_OrthoCamera.GetRotation();
		float moveSpeed = 0.01f;
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
			m_OrthoCamera.SetRotation(currentRotation + -1.0f);
		}
		if (PaulEngine::Input::IsKeyPressed(PE_KEY_PERIOD)) {
			m_OrthoCamera.SetRotation(currentRotation + 1.0f);
		}

		PaulEngine::RenderCommand::SetViewport({ 0, 0 }, { PaulEngine::Application::Get().GetWindow().GetWidth(), PaulEngine::Application::Get().GetWindow().GetHeight()});

		PaulEngine::RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		PaulEngine::RenderCommand::Clear();

		PaulEngine::Renderer::BeginScene(m_OrthoCamera);

		PaulEngine::Renderer::Submit(m_Shader2, m_SquareVertexArray);

		PaulEngine::Renderer::Submit(m_Shader, m_VertexArray);

		PaulEngine::Renderer::EndScene();
	}

	void OnImGuiRender() override {

	}

	void OnEvent(PaulEngine::Event& e) override {

	}

private:
	std::shared_ptr<PaulEngine::Shader> m_Shader;
	std::shared_ptr<PaulEngine::Shader> m_Shader2;
	std::shared_ptr<PaulEngine::VertexArray> m_VertexArray;
	std::shared_ptr<PaulEngine::VertexArray> m_SquareVertexArray;

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