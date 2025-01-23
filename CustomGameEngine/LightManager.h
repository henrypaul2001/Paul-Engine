#pragma once
#include <vector>
#include "Shader.h"
#include "Camera.h"
#include "ComponentLight.h"
#include <map>
#include <unordered_map>
#include "ResourceManager.h"
#include <stdexcept>
#include "EntityManagerNew.h"
namespace Engine {
	class LightManager
	{
	public:
		LightManager() {
			textureSlots = &ResourceManager::GetInstance()->GetTextureSlotLookupMap();
		}
		~LightManager() {}

		void SetShaderUniforms(EntityManagerNew& ecs, Shader* shader, Camera* activeCamera);
		void ResetLights() { 
			directionalLightEntities.clear();
			lightEntities.clear();
		}

		void RegisterLightEntity(float distanceToCamera, const unsigned int entityID, const ComponentLight& lightComponent) {
			if (lightComponent.GetLightType() == DIRECTIONAL) {
				directionalLightEntities.push_back(entityID);
			}
			else {
				float increment = 0.01f;
				int iterations = 0;
				while (lightEntities.find(distanceToCamera) != lightEntities.end()) {
					distanceToCamera += increment;
					increment *= 1.05f;
					iterations++;
					if (iterations >= 100) {
						throw std::length_error("Unable to increment to unique light distance from camera in suitable amount of iterations");
					}
				}
				lightEntities[distanceToCamera] = entityID;
			}
		}

		const std::vector<unsigned int>& GetDirectionalLightEntities() const { return directionalLightEntities; }
		const std::map<float, unsigned int>& GetLightEntities() const { return lightEntities; }

	private:
		std::vector<unsigned int> directionalLightEntities; // Entities with a directional light component
		std::map<float, unsigned int> lightEntities; // Entities with a light component that are either SPOT or POINT. <distanceToCamera, entityID>

		//std::vector<Entity*> lightEntities;
		//Entity* directionalLight;

		void SetDirectionalLightUniforms(EntityManagerNew& ecs, Shader* shader);
		void SetIBLUniforms(Shader* shader, Camera* activeCamera);

		const std::unordered_map<std::string, unsigned int>* textureSlots;
	};
}