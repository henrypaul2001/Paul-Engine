#include "pepch.h"
#include "Application.h"

#include "Renderer/Renderer.h"
#include "KeyCodes.h"
#include "Input.h"

namespace PaulEngine {

	Application* Application::s_Instance = nullptr;

	Application::Application() : m_OrthoCamera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		PE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(PE_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// Temporary opengl test code before moving to abstracted classes

		m_VertexArray.reset(VertexArray::Create());

		// Vertex buffer

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f,		0.8f, 0.3f, 0.2f, 1.0f,
			0.5, -0.5f, 0.0f,		0.2f, 0.8f, 0.3f, 1.0f,
			0.0f, 0.5f, 0.0f,		0.3f, 0.2f, 0.8f, 1.0f
		};

		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position", false },
			{ ShaderDataType::Float4, "a_Colour", true }
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		// Index buffer
		uint32_t indices [3] = { 0, 1, 2 };
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, 3));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		// Square
		// ------
		float squareVertices[4 * 3] = {
			-0.5f, -0.5f, 0.0f,
			0.5, -0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f
		};

		m_SquareVertexArray.reset(VertexArray::Create());
		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position", false }
		});
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		uint32_t square_indices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(square_indices, 6));
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
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
		m_Shader2.reset(new Shader(vertexSrc2, fragmentSrc2));

		//m_OrthoCamera.SetPosition(glm::vec3(-1.0f, 0.0f, 0.0f));
	}

	Application::~Application()
	{

	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher = EventDispatcher(e);
		dispatcher.DispatchEvent<WindowCloseEvent>(PE_BIND_EVENT_FN(Application::OnWindowClosed));
		//PE_CORE_INFO(e);

		// Events propagate down the layer stack, starting with overlays until the event is handled or all layers have received the event
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled()) { break; }
		}
	}

	void Application::Run()
	{
		while (m_Running) {
			//m_OrthoCamera.SetPosition(m_OrthoCamera.GetPosition() + glm::vec3(0.001f, 0.0f, 0.0f));
			const glm::vec3& currentPosition = m_OrthoCamera.GetPosition();
			const float currentRotation = m_OrthoCamera.GetRotation();
			float moveSpeed = 0.01f;
			if (Input::IsKeyPressed(PE_KEY_UP)) {
				m_OrthoCamera.SetPosition(currentPosition + glm::vec3(0.0f, moveSpeed, 0.0f));
			}
			if (Input::IsKeyPressed(PE_KEY_DOWN)) {
				m_OrthoCamera.SetPosition(currentPosition + glm::vec3(0.0f, -moveSpeed, 0.0f));
			}
			if (Input::IsKeyPressed(PE_KEY_LEFT)) {
				m_OrthoCamera.SetPosition(currentPosition + glm::vec3(-moveSpeed, 0.0f, 0.0f));
			}
			if (Input::IsKeyPressed(PE_KEY_RIGHT)) {
				m_OrthoCamera.SetPosition(currentPosition + glm::vec3(moveSpeed, 0.0f, 0.0f));
			}
			if (Input::IsKeyPressed(PE_KEY_COMMA)) {
				m_OrthoCamera.SetRotation(currentRotation + -1.0f);
			}
			if (Input::IsKeyPressed(PE_KEY_PERIOD)) {
				m_OrthoCamera.SetRotation(currentRotation + 1.0f);
			}

			RenderCommand::SetViewport({ 0, 0 }, { m_Window->GetWidth(), m_Window->GetHeight() });

			RenderCommand::SetClearColour(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			RenderCommand::Clear();

			Renderer::BeginScene(m_OrthoCamera);

			Renderer::Submit(m_Shader2, m_SquareVertexArray);

			Renderer::Submit(m_Shader, m_VertexArray);

			Renderer::EndScene();

			// Update layers
			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate();
			}

			// OnImGuiRender
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) {
				layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}