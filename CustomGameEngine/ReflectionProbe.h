#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glad/glad.h>
namespace Engine {

	struct ReflectionProbeEnvironmentMap {
		unsigned int cubemapID;
		// irradiance id
		// irradianceID;
		// prefilterID;
		// brdf_lutID;
	};

	class ReflectionProbe
	{
	public:
		ReflectionProbe(const unsigned int id, const glm::vec3& position, const unsigned int faceResWidth = 800, const unsigned int faceResHeight = 800, float nearClip = 1.0f, float farClip = 150.0f);
		~ReflectionProbe();

		const ReflectionProbeEnvironmentMap& GetProbeEnvMap() const { return envMap; }
		const glm::vec3& GetWorldPosition() const { return worldPosition; }

		const unsigned int GetFaceWidth() const { return faceWidth; }
		const unsigned int GetFaceHeight() const { return faceHeight; }
		const unsigned int GetFileID() const { return fileID; }

		const float GetNearClip() const { return nearClip; }
		const float GetFarClip() const { return farClip; }

	private:
		unsigned int fileID;
		glm::vec3 worldPosition;

		unsigned int faceWidth;
		unsigned int faceHeight;

		float nearClip;
		float farClip;

		ReflectionProbeEnvironmentMap envMap;

		void SetupCubemapTexture();
	};
}