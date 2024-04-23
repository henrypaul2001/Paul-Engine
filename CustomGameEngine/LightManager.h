#pragma once
#include <vector>
#include "Entity.h"
#include "Shader.h"
#include "Camera.h"
namespace Engine {
	class LightManager
	{
	private:
		LightManager();
		static LightManager* instance;

		std::vector<Entity*> lightEntities;
		Entity* directionalLight;

		void SetDirectionalLightUniforms(Shader* shader, Entity* directionalLight);
		void SetIBLUniforms(Shader* shader, Camera* activeCamera);
	public:
		LightManager(LightManager& other) = delete; // singleton should not be cloneable
		void operator=(const LightManager&) = delete; // singleton should not be assignable

		~LightManager();

		static LightManager* GetInstance();
		void AddLightEntity(Entity* entity);
		void SetDirectionalLightEntity(Entity* entity);
		void RemoveLightEntity(Entity* entity);

		void SetShaderUniforms(Shader* shader, Camera* activeCamera);

		std::vector<Entity*> GetLightEntities() { return lightEntities; }
		Entity* GetDirectionalLightEntity() { return directionalLight; }
	};
}