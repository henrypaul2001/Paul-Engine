#include "ReflectionProbe.h"
#include "RenderManager.h"
namespace Engine {
	ReflectionProbe::ReflectionProbe(const unsigned int id, const glm::vec3& position, const std::string& sceneName, AABBPoints localGeometryBounds, float soiRadius, const unsigned int faceResWidth, const unsigned int faceResHeight, float nearClip, float farClip, bool renderSkybox)
	{
		this->fileID = id;
		this->worldPosition = position;
		this->faceHeight = faceResHeight;
		this->faceWidth = faceResWidth;
		this->nearClip = nearClip;
		this->farClip = farClip;
		this->sceneName = sceneName;
		this->localGeometryBounds = localGeometryBounds;
		this->sphereOfInfluenceRadius = soiRadius;
		this->renderSkybox = renderSkybox;

		// ------ Set up base cubemap ------
		// ---------------------------------
		glGenTextures(1, &cubemapCaptureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapCaptureID);

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

	ReflectionProbe::~ReflectionProbe()
	{
		glDeleteTextures(1, &cubemapCaptureID);
	}
}