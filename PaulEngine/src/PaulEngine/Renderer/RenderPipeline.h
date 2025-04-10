#pragma once
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Asset/Asset.h"

#include "VertexArray.h"

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
		static Ref<RenderPipeline> Create(FaceCulling cullState, DepthState depthState, AssetHandle material);

		RenderPipeline(FaceCulling cullState, DepthState depthState, AssetHandle material) : m_CullState(cullState), m_DepthState(depthState), m_MaterialHandle(material) {}

		virtual void Bind() const = 0;

		const RenderPass& GetRenderPass() const { return m_RenderPass; }
		RenderPass& GetRenderPass() { return m_RenderPass; }
		const FaceCulling& GetCullState() const { return m_CullState; }
		const DepthState& GetDepthState() const { return m_DepthState; }

	protected:
		RenderPass m_RenderPass;
		
		const AssetHandle m_MaterialHandle;
		const FaceCulling m_CullState;
		const DepthState m_DepthState;
	};
}