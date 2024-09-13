#pragma once
#include <string>
#include <glm/ext/vector_float4.hpp>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
namespace Engine {
	enum ShaderSetupStatus {
		SETUP_NOT_STARTED,
		MISSING_FILE,
		COMPILATION_FAILED,
		COMPILED_NOT_LINKED,
		LINKED,
	};

	class AbstractShader
	{
	public:
		AbstractShader() {
			ID = 0;
			setupStatus = SETUP_NOT_STARTED;
		}
		virtual ~AbstractShader() {
			glDeleteProgram(ID);
		}

		const unsigned int GetID() const { return ID; }
		const ShaderSetupStatus GetSetupStatus() const { return setupStatus; }
		void Use() const { glUseProgram(ID); }
		void setBool(const std::string& name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
		void setInt(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
		void setUInt(const std::string& name, unsigned int value) const { glUniform1ui(glGetUniformLocation(ID, name.c_str()), value); }
		void setFloat(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
		void setMat4(const std::string& name, glm::mat4 value) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); }
		void setMat3(const std::string& name, glm::mat3 value) const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); }
		void setVec2(const std::string& name, glm::vec2 value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
		void setVec2(const std::string& name, float x, float y) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
		void setVec3(const std::string& name, glm::vec3 value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
		void setVec3(const std::string& name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }
		void setVec4(const std::string& name, glm::vec4 value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
		void setVec4(const std::string& name, float x, float y, float z, float w) const { glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); }

	protected:
		unsigned int ID;
		ShaderSetupStatus setupStatus;
	};
}