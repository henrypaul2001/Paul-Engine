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
				glCullFace(GL_FRONT);
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

	void OpenGLRenderPipeline::Bind() const
	{
		OpenGLDepthTest(m_DepthState.Test);
		glDepthMask(m_DepthState.Write);
		glDepthFunc(DepthFuncToGLEnum(m_DepthState.Func));

		OpenGLFaceCulling(m_CullState);

		AssetManager::GetAsset<Material>(m_MaterialHandle)->Bind();
	}
}