#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentLight.h"
#include "LightManager.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	class SystemLighting : public System {
	public:
		SystemLighting(EntityManager* ecs, LightManager* lightManager, Camera* activeCamera) : System(ecs), lightManager(lightManager), activeCamera(activeCamera) {}
		~SystemLighting() {}

		constexpr const char* SystemName() override { return "SYSTEM_LIGHTING"; }

		void PreAction() {
			lightManager->ResetLights();
		}

		void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentLight& light) {
			const LightTypes& type = light.GetLightType();

			if (type == SPOT) {
				// Rotate light direction based on transform matrix
				glm::mat4 model = glm::mat4(glm::mat3(transform.GetWorldModelMatrix()));
				glm::vec4 rotatedDirection4 = model * glm::vec4(light.Direction, 1.0);

				glm::vec3 rotatedDirection = glm::vec3(rotatedDirection4);
				rotatedDirection = glm::normalize(rotatedDirection);
				light.WorldDirection = rotatedDirection;
			}

			float distanceToCameraSquared = glm::distance2(activeCamera->GetPosition(), transform.GetWorldPosition());
			lightManager->RegisterLightEntity(distanceToCameraSquared, entityID, light);
		}

		void AfterAction() {}

		void SetActiveCamera(Camera* newCamera) { activeCamera = newCamera; }

	private:
		LightManager* lightManager;
		Camera* activeCamera;
	};
}