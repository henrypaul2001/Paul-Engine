#include "LightManager.h"
#include "ComponentLight.h"
#include "ComponentTransform.h"
#include "RenderManager.h"
namespace Engine {
	LightManager* LightManager::instance = nullptr;
	LightManager::LightManager() {
		directionalLight = nullptr;
		lightEntities = std::vector<Entity*>();
	}

	LightManager::~LightManager() {
		delete instance;
	}

	LightManager* LightManager::GetInstance()
	{
		if (instance == nullptr) {
			instance = new LightManager();
		}

		return instance;
	}

	void LightManager::AddLightEntity(Entity* entity)
	{
		//ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(entity->GetComponent(COMPONENT_LIGHT));
		lightEntities.push_back(entity);
		//DepthMapType type = lightComponent->GetLightType() == POINT ? MAP_CUBE : MAP_2D;
		//RenderManager::GetInstance(1024, 1024)->AssignLightToShadowMap(lightEntities.size() - 1, type);
	}

	void LightManager::SetDirectionalLightEntity(Entity* entity)
	{
		directionalLight = entity;
	}

	void LightManager::RemoveLightEntity(Entity* entity)
	{
		for (int i = 0; i < lightEntities.size(); i++) {
			if (lightEntities[i] == entity) {
				lightEntities.erase(lightEntities.begin() + i);
				return;
			}
		}
	}

	void LightManager::SetDirectionalLightUniforms(Shader* shader, Entity* directionalLight)
	{
		ComponentLight* directional = dynamic_cast<ComponentLight*>(directionalLight->GetComponent(COMPONENT_LIGHT));
		shader->setVec3("dirLight.Direction", directional->Direction);
		shader->setVec3("dirLight.Colour", directional->Colour);
		shader->setVec3("dirLight.Specular", directional->Specular);
		shader->setVec3("dirLight.Ambient", directional->Ambient);
		shader->setFloat("dirLight.LightDistance", directional->DirectionalLightDistance);
		shader->setBool("dirLight.CastShadows", directional->CastShadows);
		shader->setFloat("dirLight.MinShadowBias", directional->MinShadowBias);
		shader->setFloat("dirLight.MaxShadowBias", directional->MaxShadowBias);

		glm::vec3 lightPos = -directional->Direction * directional->DirectionalLightDistance; // negative of the directional light's direction

		glm::mat4 lightProjection = glm::ortho(-directional->ShadowProjectionSize, directional->ShadowProjectionSize, -directional->ShadowProjectionSize, directional->ShadowProjectionSize, directional->Near, directional->Far);
		glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		shader->setMat4("dirLight.LightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *RenderManager::GetInstance(1024, 1024)->GetDepthMap(-1, MAP_2D));
	}

	void LightManager::SetShaderUniforms(Shader* shader)
	{
		shader->Use();

		// First set directional light
		if (directionalLight != nullptr) {
			SetDirectionalLightUniforms(shader, directionalLight);
		}

		shader->setInt("activeLights", lightEntities.size());

		// Now spot and point lights
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
			ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));

			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Position")), transformComponent->GetWorldPosition()); // this should be transformed by the world space model matrix
			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Colour")), lightComponent->Colour);
			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Specular")), lightComponent->Specular);
			shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Ambient")), lightComponent->Ambient);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Constant")), lightComponent->Constant);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Linear")), lightComponent->Linear);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Quadratic")), lightComponent->Quadratic);
			shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].CastShadows")), lightComponent->CastShadows);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].MinShadowBias")), lightComponent->MinShadowBias);
			shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].MaxShadowBias")), lightComponent->MaxShadowBias);

			if (lightComponent->GetLightType() == POINT) {
				glActiveTexture(GL_TEXTURE0 + i + 9);
				glBindTexture(GL_TEXTURE_CUBE_MAP, *RenderManager::GetInstance(1024, 1024)->GetDepthMap(i, MAP_CUBE));
				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].SpotLight")), false);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].ShadowFarPlane")), lightComponent->Far);
			}
			else if (lightComponent->GetLightType() == SPOT) {
				glActiveTexture(GL_TEXTURE0 + i + 1);
				glBindTexture(GL_TEXTURE_2D, *RenderManager::GetInstance(1024, 1024)->GetDepthMap(i, MAP_2D));
				
				float aspect = (float)RenderManager::GetInstance(1024, 1024)->ShadowWidth() / (float)RenderManager::GetInstance(1024, 1024)->ShadowHeight();
				glm::vec3 lightPos = transformComponent->GetWorldPosition();
				glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
				glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightComponent->Direction, glm::vec3(0.0f, 1.0f, 0.0f));
				glm::mat4 lightSpaceMatrix = lightProjection * lightView;

				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].SpotLight")), true);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Direction")), lightComponent->Direction); // so should this
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Cutoff")), lightComponent->Cutoff);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].OuterCutoff")), lightComponent->OuterCutoff);
				shader->setMat4(std::string("lights[" + std::string(std::to_string(i)) + std::string("].LightSpaceMatrix")), lightSpaceMatrix);
			}
		}
	}
}