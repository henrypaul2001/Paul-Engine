#include "SystemManager.h"
#include "RenderManager.h"
#include "LightManager.h"
#include "ComponentLight.h"
#include "ResourceManager.h"
namespace Engine
{
	SystemManager::SystemManager() 
	{

	}

	SystemManager::~SystemManager()
	{
		for (System* s : updateSystemList) {
			delete s;
		}

		for (System* s : renderSystemList) {
			delete s;
		}
	}

	void SystemManager::ActionUpdateSystems(EntityManager* entityManager)
	{
		std::vector<Entity*> entityList = entityManager->Entities();
		for (System* s : updateSystemList) {
			for (Entity* e : entityList) {
				s->OnAction(e);
			}
			s->AfterAction();
		}
	}

	void SystemManager::ActionRenderSystems(EntityManager* entityManager, int SCR_WIDTH, int SCR_HEIGHT)
	{
		std::vector<Entity*> entityList = entityManager->Entities();
		//glDisable(GL_CULL_FACE);
		// shadow mapping
		if (shadowmapSystem != nullptr) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			// Directional light
			Shader* depthShader = ResourceManager::GetInstance()->ShadowMapShader();
			RenderManager* renderInstance = RenderManager::GetInstance(1024, 1024);
			renderInstance->BindShadowMapTextureToFramebuffer(-1); // bind the dir light shadowmap to framebuffer
			unsigned int* depthMapFBO = renderInstance->GetDepthFBO();
			unsigned int shadowWidth = renderInstance->ShadowWidth(); // in future, these will be stored in the light component
			unsigned int shadowHeight = renderInstance->ShadowHeight(); // <--/

			ComponentLight* dirLight = dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT));
			glm::vec3 lightPos = -dirLight->Direction * dirLight->DirectionalLightDistance; // negative of the directional light's direction
			float orthoSize = dirLight->ShadowProjectionSize;
			float near = dirLight->Near;
			float far = dirLight->Far;
			glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near, far);
			glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;

			depthShader->Use();
			depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

			glViewport(0, 0, shadowWidth, shadowHeight);
			glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			for (Entity* e : entityList) {
				shadowmapSystem->OnAction(e);
			}
			shadowmapSystem->AfterAction();

			// Spot lights
			float aspect = (float)shadowWidth / (float)shadowHeight;
			std::vector<Entity*> lightEntities = LightManager::GetInstance()->GetLightEntities();
			for (int i = 0; i < lightEntities.size(); i++) {
				ComponentLight* lightComponent = dynamic_cast<ComponentLight*>(lightEntities[i]->GetComponent(COMPONENT_LIGHT));
				ComponentTransform* transformComponent = dynamic_cast<ComponentTransform*>(lightEntities[i]->GetComponent(COMPONENT_TRANSFORM));

				if (lightComponent->GetLightType() == SPOT) {
					renderInstance->BindShadowMapTextureToFramebuffer(i);

					lightPos = transformComponent->GetWorldPosition();
					lightProjection = glm::perspective(glm::radians(90.0f), aspect, lightComponent->Near, lightComponent->Far);
					lightView = glm::lookAt(lightPos, lightPos + lightComponent->Direction, glm::vec3(0.0f, 1.0f, 0.0f));
					lightSpaceMatrix = lightProjection * lightView;

					depthShader->Use();
					depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

					glViewport(0, 0, shadowWidth, shadowHeight);
					glBindFramebuffer(GL_FRAMEBUFFER, *depthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);

					//glEnable(GL_CULL_FACE);
					//glCullFace(GL_FRONT);
					for (Entity* e : entityList) {
						shadowmapSystem->OnAction(e);
					}
					shadowmapSystem->AfterAction();
				}
			}

		}

		// render scene
		if (renderSystem != nullptr) {
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glCullFace(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (Entity* e : entityList) {
				renderSystem->OnAction(e);
			}
			renderSystem->AfterAction();
		}
	}

	void SystemManager::AddSystem(System* system, SystemLists list)
	{
		System* result = FindSystem(system->Name(), list);
		_ASSERT(result == nullptr, "System '" + system.Name() + "' already exists");
		if (list == UPDATE_SYSTEMS) {
			updateSystemList.push_back(system);
		}
		else if (list == RENDER_SYSTEMS) {
			renderSystemList.push_back(system);
			if (system->Name() == SYSTEM_RENDER) {
				renderSystem = dynamic_cast<SystemRender*>(system);
			}
			else if (system->Name() == SYSTEM_SHADOWMAP) {
				shadowmapSystem = dynamic_cast<SystemShadowMapping*>(system);
			}
		}
	}

	System* SystemManager::FindSystem(SystemTypes name, SystemLists list)
	{
		std::vector<System*>* search = nullptr;
		if (list == RENDER_SYSTEMS) {
			search = &renderSystemList;
		}
		else if (list == UPDATE_SYSTEMS) {
			search = &updateSystemList;
		}
		else {
			return nullptr;
		}

		for (System* s : *search) {
			if (s->Name() == name) {
				return s;
			}
		}

		return nullptr;
	}
}
