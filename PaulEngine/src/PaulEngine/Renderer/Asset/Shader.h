#pragma once
#include "PaulEngine/Core/Log.h"
#include "PaulEngine/Asset/Asset.h"
#include "PaulEngine/Renderer/Resource/ShaderParameterType.h"

#include <string>
#include <glm/glm.hpp>

namespace PaulEngine {
	enum class RenderPipelineContext
	{
		Undefined = 0,
		Forward,
		Deferred
	};

	static RenderPipelineContext RenderPipelineContextFromString(const std::string& contextString)
	{
		if (contextString == "forward" || contextString == "f") { return RenderPipelineContext::Forward; }
		if (contextString == "deferred" || contextString == "d") { return RenderPipelineContext::Deferred; }
		PE_CORE_ERROR("Undefined render context '{0}'", contextString);
		return RenderPipelineContext::Undefined;
	}
	static std::string RenderPipelineContextToString(RenderPipelineContext context)
	{
		switch (context)
		{
		case RenderPipelineContext::Undefined:
			return "undefined";
		case RenderPipelineContext::Forward:
			return "forward";
		case RenderPipelineContext::Deferred:
			return "deferred";
		}
	}

	class Shader : public Asset
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual RenderPipelineContext GetRenderPipelineContext() const = 0;

		virtual void SetUniformInt(const std::string& name, const int value) = 0;
		virtual void SetUniformIntArray(const std::string& name, const int* values, uint32_t count) = 0;

		virtual void SetUniformFloat(const std::string& name, const float value) = 0;
		virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) = 0;

		virtual void SetUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void SetUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

		virtual const std::string& GetName() const = 0;

		virtual const std::vector<Ref<ShaderParameterTypeSpecificationBase>>& GetReflectionData() const = 0;

		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, RenderPipelineContext shaderContext);
		static Ref<Shader> Create(const std::string& filepath);

		virtual AssetType GetType() const override { return AssetType::Shader; }
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}