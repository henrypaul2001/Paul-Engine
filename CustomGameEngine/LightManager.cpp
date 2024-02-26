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
		lightEntities.push_back(entity);
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

	void LightManager::SetShaderUniforms(Shader* shader)
	{
		shader->Use();

		// First set directional light
		if (directionalLight != nullptr) {
			ComponentLight* directional = dynamic_cast<ComponentLight*>(directionalLight->GetComponent(COMPONENT_LIGHT));
			shader->setVec3("dirLight.Direction", directional->Direction);
			shader->setVec3("dirLight.Colour", directional->Colour);
			shader->setVec3("dirLight.Specular", directional->Specular);
			shader->setVec3("dirLight.Ambient", directional->Ambient);

			glm::vec3 lightPos = -directional->Direction; // negative of the directional light's direction
			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 15.0f);
			glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;
			shader->setMat4("dirLight.LightSpaceMatrix", lightSpaceMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, *RenderManager::GetInstance()->GetDepthMap());
		}

		shader->setInt("activeLights", lightEntities.size());

		// Now spot and point lights
		for (int i = 0; i < lightEntities.size() && i < 8; i++) {
			ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
			ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));

			if (lightComponent->GetLightType() == POINT) {
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Position")), transformComponent->GetWorldPosition());
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Colour")), lightComponent->Colour);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Specular")), lightComponent->Specular);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Ambient")), lightComponent->Ambient);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Constant")), lightComponent->Constant);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Linear")), lightComponent->Linear);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Quadratic")), lightComponent->Quadratic);
				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].SpotLight")), false);
			}
			else if (lightComponent->GetLightType() == SPOT) {
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Position")), transformComponent->GetWorldPosition()); // this should be transformed by the world space model matrix
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Colour")), lightComponent->Colour);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Specular")), lightComponent->Specular);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Ambient")), lightComponent->Ambient);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Constant")), lightComponent->Constant);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Linear")), lightComponent->Linear);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Quadratic")), lightComponent->Quadratic);
				shader->setBool(std::string("lights[" + std::string(std::to_string(i)) + std::string("].SpotLight")), true);
				shader->setVec3(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Direction")), lightComponent->Direction); // so should this
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].Cutoff")), lightComponent->Cutoff);
				shader->setFloat(std::string("lights[" + std::string(std::to_string(i)) + std::string("].OuterCutoff")), lightComponent->OuterCutoff);
			}
		}
	}
}