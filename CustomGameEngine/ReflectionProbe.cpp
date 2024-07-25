#include "ReflectionProbe.h"
namespace Engine {
	ReflectionProbe::ReflectionProbe(unsigned int id, const glm::vec3& position, const unsigned int faceResWidth, const unsigned int faceResHeight, float nearClip, float farClip)
	{
		this->fileID = id;
		this->worldPosition = position;
		this->faceHeight = faceResHeight;
		this->faceWidth = faceResWidth;
		this->nearClip = nearClip;
		this->farClip = farClip;

		SetupCubemapTexture();
	}

	ReflectionProbe::~ReflectionProbe()
	{
		glDeleteTextures(1, &envMap.cubemapID);
	}

	void ReflectionProbe::SetupCubemapTexture()
	{
		glGenTextures(1, &envMap.cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.cubemapID);

		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, faceWidth, faceHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}