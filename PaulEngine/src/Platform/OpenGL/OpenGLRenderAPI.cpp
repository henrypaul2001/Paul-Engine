#include "pepch.h"
#include "OpenGLRenderAPI.h"

#include <glad/glad.h>
namespace PaulEngine {
	OpenGLRenderAPI::OpenGLRenderAPI() {}
	OpenGLRenderAPI::~OpenGLRenderAPI() {}

	void OpenGLRenderAPI::Init()
	{
		PE_PROFILE_FUNCTION();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRenderAPI::SetViewport(const glm::ivec2& position, const glm::ivec2& size)
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

	void OpenGLRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount)
	{
		PE_PROFILE_FUNCTION();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderAPI::DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount)
	{
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRenderAPI::SetLineWidth(const float thickness)
	{
		glLineWidth(thickness);
	}
}