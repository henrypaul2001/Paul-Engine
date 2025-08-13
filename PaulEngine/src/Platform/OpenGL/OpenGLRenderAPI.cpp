#include "pepch.h"
#include "OpenGLRenderAPI.h"
#include "PaulEngine/Renderer/Resource/Framebuffer.h"

#include <glad/glad.h>
namespace PaulEngine {

	constexpr GLenum DepthFuncToGLEnum(DepthFunc func) {
		switch (func)
		{
		case DepthFunc::NEVER:
			return GL_NEVER;
		case DepthFunc::LESS:
			return GL_LESS;
		case DepthFunc::LEQUAL:
			return GL_LEQUAL;
		case DepthFunc::EQUAL:
			return GL_EQUAL;
		case DepthFunc::GREATER:
			return GL_GREATER;
		case DepthFunc::GEQUAL:
			return GL_GEQUAL;
		case DepthFunc::NEQUAL:
			return GL_NOTEQUAL;
		case DepthFunc::ALWAYS:
			return GL_ALWAYS;
		}
		return GL_NONE;
	}

	constexpr void OpenGLFaceCulling(const FaceCulling culling) {
		switch (culling)
		{
		case FaceCulling::NONE:
			glDisable(GL_CULL_FACE);
			return;
		case FaceCulling::FRONT:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			return;
		case FaceCulling::BACK:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			return;
		case FaceCulling::FRONT_AND_BACK:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT_AND_BACK);
			return;
		}
	}

	constexpr GLenum BlendFuncToGLEnum(BlendFunc func)
	{
		switch (func)
		{
		case BlendFunc::ZERO:
			return GL_ZERO;
		case BlendFunc::ONE:
			return GL_ONE;
		case BlendFunc::SRC_COLOUR:
			return GL_SRC_COLOR;
		case BlendFunc::ONE_MINUS_SRC_COLOUR:
			return GL_ONE_MINUS_SRC_COLOR;
		case BlendFunc::DST_COLOUR:
			return GL_DST_COLOR;
		case BlendFunc::ONE_MINUS_DST_COLOUR:
			return GL_ONE_MINUS_DST_COLOR;
		case BlendFunc::SRC_ALPHA:
			return GL_SRC_ALPHA;
		case BlendFunc::ONE_MINUS_SRC_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFunc::DST_ALPHA:
			return GL_DST_ALPHA;
		case BlendFunc::ONE_MINUS_DST_ALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case BlendFunc::CONSTANT_COLOUR:
			return GL_CONSTANT_COLOR;
		case BlendFunc::ONE_MINUS_CONSTANT_COLOUR:
			return GL_ONE_MINUS_CONSTANT_COLOR;
		case BlendFunc::CONSTANT_ALPHA:
			return GL_CONSTANT_ALPHA;
		case BlendFunc::ONE_MINUS_CONSTANT_ALPHA:
			return GL_ONE_MINUS_CONSTANT_ALPHA;
		}
		return GL_ONE;
	}

	constexpr GLenum BlendEquationToGLEnum(BlendEquation func)
	{
		switch (func)
		{
		case BlendEquation::ADD:
			return GL_FUNC_ADD;
		case BlendEquation::SUBTRACT:
			return GL_FUNC_SUBTRACT;
		case BlendEquation::REVERSE_SUBTRACT:
			return GL_FUNC_REVERSE_SUBTRACT;
		case BlendEquation::MIN:
			return GL_MIN;
		case BlendEquation::MAX:
			return GL_MAX;
		}
		return GL_FUNC_ADD;
	}

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

	void OpenGLRenderAPI::EnableDepth()
	{
		glEnable(GL_DEPTH_TEST);
	}

	void OpenGLRenderAPI::DisableDepth()
	{
		glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRenderAPI::DepthMask(bool write)
	{
		glDepthMask(write);
	}

	void OpenGLRenderAPI::DepthFunc(PaulEngine::DepthFunc func)
	{
		glDepthFunc(DepthFuncToGLEnum(func));
	}

	void OpenGLRenderAPI::SetFaceCulling(FaceCulling cullState)
	{
		OpenGLFaceCulling(cullState);
	}

	void OpenGLRenderAPI::EnableBlend()
	{
		glEnable(GL_BLEND);
	}

	void OpenGLRenderAPI::DisableBlend()
	{
		glDisable(GL_BLEND);
	}

	void OpenGLRenderAPI::BlendFunc(PaulEngine::BlendFunc srcFactor, PaulEngine::BlendFunc dstFactor)
	{
		glBlendFunc(BlendFuncToGLEnum(srcFactor), BlendFuncToGLEnum(dstFactor));
	}

	void OpenGLRenderAPI::BlendColour(glm::vec4 constantColour)
	{
		glBlendColor(constantColour.r, constantColour.g, constantColour.b, constantColour.a);
	}

	void OpenGLRenderAPI::BlendEquation(PaulEngine::BlendEquation blendEquation)
	{
		glBlendEquation(BlendEquationToGLEnum(blendEquation));
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