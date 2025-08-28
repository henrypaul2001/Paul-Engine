#pragma once
#include "PaulEngine/Utils/Hash.h"
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Asset/Asset.h"

#include "Resource/VertexArray.h"

#include <glm/glm.hpp>
namespace PaulEngine
{
	struct DrawSubmission
	{
		AssetHandle MeshHandle;
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

	enum class DepthFunc : uint8_t
	{
		NEVER = 0,
		LESS, LEQUAL,
		EQUAL,
		GREATER, GEQUAL,
		NEQUAL,
		ALWAYS
	};
	struct DepthState
	{
		DepthFunc Func = DepthFunc::LESS;
		bool Test = true;
		bool Write = true;
	};
	inline bool operator==(const DepthState& left, const DepthState& right)
	{
		return (left.Func == right.Func &&
				left.Test == right.Test &&
				left.Write == right.Write);
	}

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
	inline bool operator==(const BlendState& left, const BlendState& right)
	{
		return (left.Enabled == right.Enabled &&
			left.SrcFactor == right.SrcFactor &&
			left.DstFactor == right.DstFactor &&
			left.Equation == right.Equation &&
			left.ConstantColour == right.ConstantColour);
	}

	struct PipelineParams
	{
		FaceCulling CullState = FaceCulling::BACK;
		DepthState DepthState = {};
		BlendState BlendState = {};
	};
	struct RenderPipelineSpecification
	{
		PipelineParams Params;
		AssetHandle MaterialHandle = 0;
	};

	using RenderPipelineHash = size_t;
	class RenderPipeline
	{
	public:
		static Ref<RenderPipeline> Create(RenderPipelineSpecification spec);

		RenderPipeline(FaceCulling cullState, DepthState depthState, BlendState blendState, AssetHandle material) : m_CullState(cullState), m_DepthState(depthState), m_BlendState(blendState), m_MaterialHandle(material) {}

		void Bind() const;

		const std::vector<DrawSubmission>& GetDrawList() const { return m_DrawList; }
		std::vector<DrawSubmission>& GetDrawList() { return m_DrawList; }
		const FaceCulling& GetCullState() const { return m_CullState; }
		const DepthState& GetDepthState() const { return m_DepthState; }
		const BlendState& GetBlendState() const { return m_BlendState; }

		RenderPipelineHash Hash() const;
		static void ResetBuffers();
	private:
		friend class std::hash<RenderPipeline>;
		std::vector<DrawSubmission> m_DrawList;
		
		const AssetHandle m_MaterialHandle;
		const FaceCulling m_CullState = FaceCulling::BACK;
		const DepthState m_DepthState;
		const BlendState m_BlendState;
	};
}

MAKE_HASHABLE(PaulEngine::RenderPipeline, t.m_MaterialHandle, t.m_CullState, t.m_DepthState, t.m_BlendState)
MAKE_HASHABLE(PaulEngine::DepthState, t.Func, t.Test, t.Write)
MAKE_HASHABLE(PaulEngine::BlendState, t.Enabled, t.SrcFactor, t.DstFactor, t.Equation, t.ConstantColour)
MAKE_HASHABLE(PaulEngine::Ref<PaulEngine::RenderPipeline>, t.get())