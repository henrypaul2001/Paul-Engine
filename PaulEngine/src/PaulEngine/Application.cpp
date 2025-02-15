#include "pepch.h"
#include "Application.h"
#include <glad/glad.h>

namespace PaulEngine {

	Application* Application::s_Instance = nullptr;

	static GLenum ShaderDataTypeToGLBaseType(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::None:		return 0;
			case ShaderDataType::Float:		return GL_FLOAT;
			case ShaderDataType::Float2:	return GL_FLOAT;
			case ShaderDataType::Float3:	return GL_FLOAT;
			case ShaderDataType::Float4:	return GL_FLOAT;
			case ShaderDataType::Mat3:		return GL_FLOAT;
			case ShaderDataType::Mat4:		return GL_FLOAT;
			case ShaderDataType::Int:		return GL_INT;
			case ShaderDataType::Int2:		return GL_INT;
			case ShaderDataType::Int3:		return GL_INT;
			case ShaderDataType::Int4:		return GL_INT;
			case ShaderDataType::Bool:		return GL_BOOL;
		}
		PE_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	Application::Application()
	{
		PE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(PE_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// Temporary opengl test code before moving to abstracted classes

		// Vertex buffer
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f,		0.8f, 0.3f, 0.2f, 1.0f,
			0.5, -0.5f, 0.0f,		0.2f, 0.8f, 0.3f, 1.0f,
			0.0f, 0.5f, 0.0f,		0.3f, 0.2f, 0.8f, 1.0f
		};

		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		//m_VertexBuffer->Bind();

		// Temp code vertex layout code
		{
			BufferLayout layout = {
				{ ShaderDataType::Float3, "a_Position", false },
				{ ShaderDataType::Float4, "a_Colour", true }
			};

			m_VertexBuffer->SetLayout(layout);
		}

		uint32_t index = 0;
		const auto& layout = m_VertexBuffer->GetLayout();
		for (const auto& element : layout) {
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index, 
				element.GetComponentCount(), 
				ShaderDataTypeToGLBaseType(element.Type), 
				element.Normalized ? GL_TRUE : GL_FALSE, 
				layout.GetStride(), 
				(const void*)element.Offset);
			index++;
		}

		// Index buffer
		uint32_t indices [3] = { 0, 1, 2 };
		m_IndexBuffer.reset(IndexBuffer::Create(indices, 3));

		std::string vertexSrc = R"(
			#version 330 core

			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec4 a_Colour;

			out vec3 v_Position;
			out vec4 v_Colour;

			void main()
			{
				v_Colour = a_Colour;
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
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

		// Shader
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
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
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

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