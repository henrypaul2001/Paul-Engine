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
		ReflectionProbe(const unsigned int id, const glm::vec3& position, const unsigned int faceResWidth = 800, const unsigned int faceResHeight = 800);
		~ReflectionProbe();

		const ReflectionProbeEnvironmentMap& GetProbeEnvMap() const { return envMap; }

	private:
		unsigned int fileID;
		glm::vec3 worldPosition;

		unsigned int faceWidth;
		unsigned int faceHeight;

		ReflectionProbeEnvironmentMap envMap;

		void SetupCubemapTexture();
	};
}