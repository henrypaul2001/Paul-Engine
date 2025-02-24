#pragma once
#include "PaulEngine/Renderer/Shader.h"
#include <glm/glm.hpp>
#include <unordered_map>

typedef unsigned int GLenum; // TODO: remove

namespace PaulEngine {
    class OpenGLShader : public Shader
    {
	public:
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		OpenGLShader(const std::string& filepath);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return m_Name; }

		virtual void SetUniformInt(const std::string& name, const int value) override;
		virtual void SetUniformIntArray(const std::string& name, const int* values, uint32_t count) override;

		virtual void SetUniformFloat(const std::string& name, const float value) override;
		virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) override;

		virtual void SetUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		virtual void SetUniformMat4(const std::string& name, const glm::mat4& matrix) override;

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
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

		uint32_t m_RendererID;
		std::string m_Name;
    };
}