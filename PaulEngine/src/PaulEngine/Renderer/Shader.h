#pragma once
#include <string>

namespace PaulEngine {
	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;

		static Shader* Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Shader* Create(const std::string& filepath);
	};
}