#include "pepch.h"
#include "OpenGLVertexArray.h"

#include <glad/gl.h>

namespace PaulEngine {

	static constexpr bool ShaderDataTypeIsInteger(ShaderDataType type) {
		return (type == ShaderDataType::Int || type == ShaderDataType::Int2 || type == ShaderDataType::Int3 || type == ShaderDataType::Int4);
	}

	static GLenum ShaderDataTypeToGLBaseType(ShaderDataType type) {
		PE_PROFILE_FUNCTION();
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

	OpenGLVertexArray::OpenGLVertexArray(DrawPrimitive drawPrimitive) : m_RendererID(0), m_Primitive(drawPrimitive)
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex buffer has no layout assigned");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			bool isInteger = ShaderDataTypeIsInteger(element.Type);
			glEnableVertexAttribArray(index);
			if (!isInteger) {
				glVertexAttribPointer(
					index,
					GetComponentCount(element.Type),
					ShaderDataTypeToGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
			}
			else {
				glVertexAttribIPointer(
					index,
					GetComponentCount(element.Type),
					ShaderDataTypeToGLBaseType(element.Type),
					layout.GetStride(),
					(const void*)element.Offset);
			}
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

	bool OpenGLVertexArray::operator==(const VertexArray* other) const
	{
		return (m_RendererID == static_cast<const OpenGLVertexArray*>(other)->m_RendererID);
	}

	bool OpenGLVertexArray::operator!=(const VertexArray* other) const
	{
		return (m_RendererID != static_cast<const OpenGLVertexArray*>(other)->m_RendererID);
	}
}