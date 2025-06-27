#include "pepch.h"
#include "OpenGLRenderAPI.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"

#include <glad/glad.h>
namespace PaulEngine {

	static GLenum DrawPrimitiveToGLEnum(const DrawPrimitive primitive)
	{
		switch (primitive)
		{
			case DrawPrimitive::None: return 0;

			case DrawPrimitive::POINTS: return GL_POINTS;

			case DrawPrimitive::LINES: return GL_LINES;
			case DrawPrimitive::LINE_LOOP: return GL_LINE_LOOP;
			case DrawPrimitive::LINE_STRIP: return GL_LINE_STRIP;
			case DrawPrimitive::LINES_ADJACENCY: return GL_LINES_ADJACENCY;
			case DrawPrimitive::LINE_STRIP_ADJACENCY: return GL_LINE_STRIP_ADJACENCY;

			case DrawPrimitive::TRIANGLES: return GL_TRIANGLES;
			case DrawPrimitive::TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
			case DrawPrimitive::TRIANGLE_FAN: return GL_TRIANGLE_FAN;
			case DrawPrimitive::TRIANGLES_ADJACENCY: return GL_TRIANGLES_ADJACENCY;
			case DrawPrimitive::TRIANGLE_STRIP_ADJACENCY: return GL_TRIANGLE_STRIP_ADJACENCY;
		}
		PE_CORE_ERROR("Undefined DrawPrimitive translation");
		return 0;
	}

	OpenGLRenderAPI::OpenGLRenderAPI() {}
	OpenGLRenderAPI::~OpenGLRenderAPI() {}

	void OpenGLRenderAPI::Init()
	{
		PE_PROFILE_FUNCTION();
		glDisable(GL_BLEND);
		glDisable(GL_DITHER);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	void OpenGLRenderAPI::SetViewport(const glm::ivec2& position, const glm::ivec2& size)
	{
		glViewport(position.x, position.y, size.x, size.y);
	}

	void OpenGLRenderAPI::SetClearColour(const glm::vec4& colour)
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void OpenGLRenderAPI::Clear(int bufferMask)
	{
		int mask = BufferBitMaskToGLBitMask(bufferMask);
		glClear(mask);
	}

	void OpenGLRenderAPI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t indexCount)
	{
		PE_PROFILE_FUNCTION();
		glDrawElements(DrawPrimitiveToGLEnum(vertexArray->GetDrawPrimitive()), indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderAPI::DrawLines(const Ref<VertexArray>& vertexArray, const uint32_t vertexCount)
	{
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRenderAPI::SetLineWidth(float thickness)
	{
		thickness = std::max(0.01f, thickness);
		glLineWidth(thickness);
	}

	int OpenGLRenderAPI::BufferBitMaskToGLBitMask(int bufferMask)
	{
		int glMask = 0;
		if (bufferMask & Framebuffer::BufferBit::COLOUR) { glMask |= GL_COLOR_BUFFER_BIT; }
		if (bufferMask & Framebuffer::BufferBit::DEPTH) { glMask |= GL_DEPTH_BUFFER_BIT; }
		if (bufferMask & Framebuffer::BufferBit::STENCIL) { glMask |= GL_STENCIL_BUFFER_BIT; }
		return glMask;
	}
}