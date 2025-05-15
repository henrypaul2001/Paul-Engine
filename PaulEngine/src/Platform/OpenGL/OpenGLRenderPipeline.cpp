#include "pepch.h"
#include "OpenGLRenderPipeline.h"
#include "PaulEngine/Asset/AssetManager.h"
#include "PaulEngine/Renderer/Material.h"

#include <glad/glad.h>

namespace PaulEngine
{
	constexpr void OpenGLDepthTest(const bool test) {
		if (test) { glEnable(GL_DEPTH_TEST); }
		else { glDisable(GL_DEPTH_TEST); }
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

/*
	struct BlendState
	{
		bool Enabled = true;
		BlendFunc SrcFactor = BlendFunc::SRC_ALPHA;
		BlendFunc DstFactor = BlendFunc::ONE_MINUS_SRC_ALPHA;
		BlendEquation Equation = BlendEquation::ADD;
		glm::vec4 ConstantColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	};
	*/

	void OpenGLRenderPipeline::Bind() const
	{
		OpenGLDepthTest(m_DepthState.Test);
		glDepthMask(m_DepthState.Write);
		glDepthFunc(DepthFuncToGLEnum(m_DepthState.Func));

		OpenGLFaceCulling(m_CullState);

		Blend(m_BlendState);

		if (AssetManager::IsAssetHandleValid(m_MaterialHandle)) {
			AssetManager::GetAsset<Material>(m_MaterialHandle)->Bind();
		}
	}

	void OpenGLRenderPipeline::Blend(BlendState blendState) const
	{
		if (blendState.Enabled)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		glBlendFunc(BlendFuncToGLEnum(blendState.SrcFactor), BlendFuncToGLEnum(blendState.DstFactor));
		glBlendColor(blendState.ConstantColour.r, blendState.ConstantColour.g, blendState.ConstantColour.b, blendState.ConstantColour.a);
		glBlendEquation(BlendEquationToGLEnum(blendState.Equation));
	}
}