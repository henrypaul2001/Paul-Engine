#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glad/glad.h>
#include <string>
#include "ComponentCollisionAABB.h"
namespace Engine {

	struct ReflectionProbeEnvironmentMap {
		unsigned int cubemapID;
		unsigned int irradianceID;
		unsigned int prefilterID;
	};

	class ReflectionProbe
	{
	public:
		ReflectionProbe(const unsigned int id, const glm::vec3& position, const std::string& sceneName, AABBPoints localGeometryBounds, float soiRadius, const unsigned int faceResWidth = 1280, const unsigned int faceResHeight = 1280, float nearClip = 1.0f, float farClip = 150.0f, bool renderSkybox = true);
		~ReflectionProbe();

		const ReflectionProbeEnvironmentMap& GetProbeEnvMapConst() const { return envMap; }
		ReflectionProbeEnvironmentMap& GetProbeEnvMap() { return envMap; }
		const glm::vec3& GetWorldPosition() const { return worldPosition; }

		const unsigned int GetFaceWidth() const { return faceWidth; }
		const unsigned int GetFaceHeight() const { return faceHeight; }
		const unsigned int GetFileID() const { return fileID; }

		const bool GetRenderSkybox() const { return renderSkybox; }

		const float GetNearClip() const { return nearClip; }
		const float GetFarClip() const { return farClip; }
		
		const float GetSOIRadius() const { return sphereOfInfluenceRadius; }

		const std::string& GetSceneName() const { return sceneName; }
		const AABBPoints& GetLocalGeometryBounds() const { return localGeometryBounds; }
	private:
		unsigned int fileID;

		unsigned int faceWidth;
		unsigned int faceHeight;

		bool renderSkybox;

		float nearClip;
		float farClip;

		float sphereOfInfluenceRadius;

		glm::vec3 worldPosition;

		std::string sceneName;

		ReflectionProbeEnvironmentMap envMap;
		AABBPoints localGeometryBounds;

		void SetupTextureMaps();
	};
}