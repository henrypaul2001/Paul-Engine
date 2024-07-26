#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glad/glad.h>
namespace Engine {

	struct ReflectionProbeEnvironmentMap {
		unsigned int cubemapID;
		unsigned int irradianceID;
		unsigned int prefilterID;
		unsigned int brdf_lutID;
	};

	class ReflectionProbe
	{
	public:
		ReflectionProbe(const unsigned int id, const glm::vec3& position, const unsigned int faceResWidth = 1280, const unsigned int faceResHeight = 1280, float nearClip = 1.0f, float farClip = 150.0f);
		~ReflectionProbe();

		const ReflectionProbeEnvironmentMap& GetProbeEnvMapConst() const { return envMap; }
		ReflectionProbeEnvironmentMap& GetProbeEnvMap() { return envMap; }
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

		void SetupTextureMaps();
		void LoadBakedReflectionProbe();
	};
}