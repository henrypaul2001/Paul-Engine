#include "pepch.h"
#include "OpenGLRenderAPI.h"

#include <glad/glad.h>
namespace PaulEngine {
	OpenGLRenderAPI::OpenGLRenderAPI() {}
	OpenGLRenderAPI::~OpenGLRenderAPI() {}

	void OpenGLRenderAPI::SetViewport(const glm::vec2& position, const glm::vec2& size)
	{
		glViewport(position.x, position.y, size.x, size.y);
	}

	void OpenGLRenderAPI::SetClearColour(const glm::vec4& colour)
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void OpenGLRenderAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}