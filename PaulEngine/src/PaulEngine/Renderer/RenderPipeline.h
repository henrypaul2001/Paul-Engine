#pragma once
#include "PaulEngine/Utils/Hash.h"
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Asset/Asset.h"

#include "Resource/VertexArray.h"
#include "Resource/ShaderStorageBuffer.h"

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

	struct MeshSubmissionData
	{
		glm::mat4 Transform;
		int32_t EntityID;
		uint32_t MaterialIndex;
		int32_t padding1;
		int32_t padding2;
	};

	enum class FaceCulling
	{
		NONE = 0,
		FRONT,
		BACK,
		FRONT_AND_BACK
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
	
	using RenderPipelineHash = size_t;
	struct PipelineParams
	{
		FaceCulling CullState = FaceCulling::BACK;
		DepthState DepthState = {};
		BlendState BlendState = {};
	};
	struct RenderPipelineSpecification
	{
		PipelineParams Params;
		AssetHandle ShaderHandle = 0;
		RenderPipelineHash Hash() const;
	};

	class RenderPipeline
	{
	public:
		static Ref<RenderPipeline> Create(RenderPipelineSpecification spec);

		RenderPipeline(FaceCulling cullState, DepthState depthState, BlendState blendState, AssetHandle shader) : m_CullState(cullState), m_DepthState(depthState), m_BlendState(blendState), m_ShaderHandle(shader) {}

		void Bind() const;

		inline const AssetHandle& GetShaderHandle() const { return m_ShaderHandle; }
		inline const FaceCulling& GetCullState() const { return m_CullState; }
		inline const DepthState& GetDepthState() const { return m_DepthState; }
		inline const BlendState& GetBlendState() const { return m_BlendState; }

		RenderPipelineHash Hash() const;

	private:
		friend class std::hash<RenderPipeline>;

		const AssetHandle m_ShaderHandle;
		const FaceCulling m_CullState = FaceCulling::BACK;
		const DepthState m_DepthState;
		const BlendState m_BlendState;
	};
}

MAKE_HASHABLE(PaulEngine::PipelineParams, t.CullState, t.DepthState, t.BlendState)
MAKE_HASHABLE(PaulEngine::RenderPipelineSpecification, t.Params, t.ShaderHandle)
MAKE_HASHABLE(PaulEngine::RenderPipeline, t.m_ShaderHandle, t.m_CullState, t.m_DepthState, t.m_BlendState)
MAKE_HASHABLE(PaulEngine::DepthState, t.Func, t.Test, t.Write)
MAKE_HASHABLE(PaulEngine::BlendState, t.Enabled, t.SrcFactor, t.DstFactor, t.Equation, t.ConstantColour)
MAKE_HASHABLE(PaulEngine::Ref<PaulEngine::RenderPipeline>, t.get())