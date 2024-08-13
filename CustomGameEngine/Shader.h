#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
namespace Engine {
	class Shader
	{
	public:
		Shader(const char* vPath, const char* fPath);
		Shader(const char* vPath, const char* fPath, const char* gPath);
		~Shader() {
			glDeleteShader(ID);
		}

		void Use();

		// uniform utility functions
		void setBool(const std::string& name, bool value) const;
		void setInt(const std::string& name, int value) const;
		void setUInt(const std::string& name, unsigned int value) const;
		void setFloat(const std::string& name, float value) const;
		void setMat4(const std::string& name, glm::mat4 value) const;
		void setMat3(const std::string& name, glm::mat3 value) const;
		void setVec2(const std::string& name, glm::vec2 value) const;
		void setVec2(const std::string& name, float x, float y) const;
		void setVec3(const std::string& name, glm::vec3 value) const;
		void setVec3(const std::string& name, float x, float y, float z) const;
		void setVec4(const std::string& name, glm::vec4 value) const;
		void setVec4(const std::string& name, float x, float y, float z, float w) const;

		unsigned int GetID() { return ID; }
	private:
		unsigned int ID;
	};
}
#endif