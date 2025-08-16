#pragma once
#include "PaulEngine/Renderer/Asset/Shader.h"
#include <glm/glm.hpp>
#include <unordered_map>

#include <spirv_reflect.h>

typedef unsigned int GLenum; // TODO: remove

namespace PaulEngine {
    class OpenGLShader : public Shader
    {
	public:
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc, RenderPipelineContext shaderContext);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, RenderPipelineContext shaderContext);
		OpenGLShader(const std::string& filepath);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual RenderPipelineContext GetRenderPipelineContext() const { return m_ShaderContext; }

		virtual const std::string& GetName() const override { return m_Name; }

		virtual void SetUniformInt(const std::string& name, const int value) override;
		virtual void SetUniformIntArray(const std::string& name, const int* values, uint32_t count) override;

		virtual void SetUniformFloat(const std::string& name, const float value) override;
		virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) override;

		virtual void SetUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMat4(const std::string& name, const glm::mat4& matrix) override;

		virtual const std::vector<Ref<ShaderParameterTypeSpecificationBase>>& GetReflectionData() const override { return m_ReflectionData; }

		void UploadUniformInt(const std::string& name, const int value);
		void UploadUniformIntArray(const std::string& name, const int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, const float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);

		void CompileOrGetOpenGLBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
		bool ReflectUBOs(spv_reflect::ShaderModule& reflection, std::string& error);
		bool ReflectSamplers(spv_reflect::ShaderModule& reflection, std::string& error);
		void ReflectBlockVariableRecursive(spv_reflect::ShaderModule& reflection, SpvReflectBlockVariable* member, const std::string& parentName = "");
		void AsVectorType(ShaderDataType& type, const SpvReflectBlockVariable* member);
		void AsMatrixType(ShaderDataType& type, const SpvReflectBlockVariable* member);
		void OverrideArrayOpType(SpvOp& op, SpvReflectTypeFlags flags);

		uint32_t m_RendererID;
		std::string m_Filepath;
		std::string m_Name;
		RenderPipelineContext m_ShaderContext;

		std::vector<Ref<ShaderParameterTypeSpecificationBase>> m_ReflectionData;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;
    };
}