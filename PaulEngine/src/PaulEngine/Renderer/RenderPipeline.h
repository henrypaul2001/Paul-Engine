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

	//struct RenderPass
	//{
	//	std::vector<DrawSubmission> DrawList;
	//	// lighting information
	//	// environment info (skybox, reflection probes, shadow maps, etc)
	//	// render target
	//	// post processing steps
	//	// ...
	//};

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
		DepthFunc Func = DepthFunc::LESS;
		bool Test = true;
		bool Write = true;
	};

	enum class BlendFunc
	{
		ZERO = 0,
		ONE,
		SRC_COLOUR,
		ONE_MINUS_SRC_COLOUR,
		DST_COLOUR,
		ONE_MINUS_DST_COLOUR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOUR,
		ONE_MINUS_CONSTANT_COLOUR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA
	};

	enum class BlendEquation
	{
		ADD = 0,
		SUBTRACT,
		REVERSE_SUBTRACT,
		MIN,
		MAX
	};

	struct BlendState
	{
		bool Enabled = true;
		BlendFunc SrcFactor = BlendFunc::SRC_ALPHA;
		BlendFunc DstFactor = BlendFunc::ONE_MINUS_SRC_ALPHA;
		BlendEquation Equation = BlendEquation::ADD;
		glm::vec4 ConstantColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	};

	class RenderPipeline
	{
	public:
		static Ref<RenderPipeline> Create(FaceCulling cullState, DepthState depthState, BlendState blendState, AssetHandle material);

		RenderPipeline(FaceCulling cullState, DepthState depthState, BlendState blendState, AssetHandle material) : m_CullState(cullState), m_DepthState(depthState), m_BlendState(blendState), m_MaterialHandle(material) {}

		virtual void Bind() const = 0;

		const std::vector<DrawSubmission>& GetDrawList() const { return m_DrawList; }
		std::vector<DrawSubmission>& GetDrawList() { return m_DrawList; }
		const FaceCulling& GetCullState() const { return m_CullState; }
		const DepthState& GetDepthState() const { return m_DepthState; }

	protected:
		std::vector<DrawSubmission> m_DrawList;
		
		const AssetHandle m_MaterialHandle;
		const FaceCulling m_CullState = FaceCulling::BACK;
		const DepthState m_DepthState;
		const BlendState m_BlendState;
	};
}