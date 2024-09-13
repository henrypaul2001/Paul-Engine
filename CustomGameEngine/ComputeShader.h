#pragma once
#include "AbstractShader.h"
namespace Engine {
    class ComputeShader : public AbstractShader
    {
    public:
        ComputeShader(const char* cPath) {
			// retrieve source code from file
			std::string computeCode;
			std::ifstream cShaderFile;

			// ensure ifstream objects can throw exceptions
			cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try {
				// open file
				cShaderFile.open(cPath);
				std::stringstream cShaderStream;

				// read files buffer contents into stream
				cShaderStream << cShaderFile.rdbuf();

				// close file
				cShaderFile.close();

				// convert stream into string
				computeCode = cShaderStream.str();
			}
			catch (std::ifstream::failure e) {
				std::cout << "ERROR::COMPUTESHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
				setupStatus = MISSING_FILE;
				return;
			}

			const char* cShaderCode = computeCode.c_str();

			//  compile shader
			unsigned int compute;
			int success;
			char infoLog[512];

			// vertex
			compute = glCreateShader(GL_COMPUTE_SHADER);
			glShaderSource(compute, 1, &cShaderCode, NULL);
			glCompileShader(compute);

			// print compile errors
			glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(compute, 512, NULL, infoLog);
				std::cout << "ERROR::COMPUTESHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
				setupStatus = COMPILATION_FAILED;
				glDeleteShader(compute);
				return;
			}

			ID = glCreateProgram();
			glAttachShader(ID, compute);
			glLinkProgram(ID);

			// print linking errors
			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(ID, 512, NULL, infoLog);
				std::cout << "ERROR::COMPUTESHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
				setupStatus = COMPILED_NOT_LINKED;
				glDeleteShader(compute);
				glDeleteProgram(ID);
				return;
			}

			// delete shader (no longer needed after being linked to program)
			glDeleteShader(compute);
        }
        ~ComputeShader() {}

		void DispatchCompute(const unsigned int xGroups = 1, const unsigned int yGroups = 1, const unsigned int zGroups = 1, GLbitfield barrierBits = GL_ALL_BARRIER_BITS) const {
			assert(xGroups >= 1, "ERROR::ComputeShader::xGroups cannot be less than one");
			assert(yGroups >= 1, "ERROR::ComputeShader::yGroups cannot be less than one");
			assert(zGroups >= 1, "ERROR::ComputeShader::zGroups cannot be less than one");

			assert(xGroups < maxWorkGroupsX, "ERROR::ComputeShader::xGroups exceeds max group count: " << maxWorkGroupsX);
			assert(yGroups < maxWorkGroupsY, "ERROR::ComputeShader::yGroups exceeds max group count: " << maxWorkGroupsY);
			assert(zGroups < maxWorkGroupsZ, "ERROR::ComputeShader::zGroups exceeds max group count: " << maxWorkGroupsZ);

			Use();
			glDispatchCompute(xGroups, yGroups, zGroups);

			glMemoryBarrier(barrierBits);
		}

		static void InitOpenGLConstants() {
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupsX);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupsY);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupsZ);
		}

	private:
		static int maxWorkGroupsX;
		static int maxWorkGroupsY;
		static int maxWorkGroupsZ;
    };
}