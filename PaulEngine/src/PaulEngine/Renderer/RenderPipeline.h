#pragma once
#include "PaulEngine/Core/Core.h"
#include "VertexArray.h"
#include "PaulEngine/Asset/Asset.h"

#include <glm/glm.hpp>
namespace PaulEngine
{
	struct DrawSubmission
	{
		Ref<VertexArray> VertexArray;
		AssetHandle MaterialHandle;
		glm::mat4 Transform;
		int EntityID;
	};

	struct RenderPass
	{
		std::vector<DrawSubmission> DrawList;
		// lighting information
		// environment info (skybox, reflection probes, shadow maps, etc)
		// render target
		// post processing steps
		// ...
	};

	enum class DepthFunc
	{
		NEVER = 0,
		LESS, LEQUAL,
		EQUAL,
		GREATER, GEQUAL,
		NEQUAL,
		ALWAYS
	};

	enum class FaceCulling
	{
		NONE = 0,
		FRONT,
		BACK,
		FRONT_AND_BACK
	};

	struct DepthState
	{
		DepthFunc Func;
		bool Test;
		bool Write;
	};

	class RenderPipeline
	{
	public:
		static Ref<RenderPipeline> Create();

		virtual void Bind() const = 0;

	private:
		RenderPass m_RenderPass;
		FaceCulling m_CullState;
		DepthState m_DepthState;
	};
}