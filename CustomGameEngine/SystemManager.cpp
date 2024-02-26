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

		// shadow mapping
		if (shadowmapSystem != nullptr) {
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_BACK);
			Shader* depthShader = ResourceManager::GetInstance()->ShadowMapShader();

			unsigned int* depthMapFBO = RenderManager::GetInstance()->GetDepthFBO();
			unsigned int shadowWidth = RenderManager::GetInstance()->ShadowWidth(); // in future, these will be stored in the light component
			unsigned int shadowHeight = RenderManager::GetInstance()->ShadowHeight(); // <--/

			ComponentLight* dirLight = dynamic_cast<ComponentLight*>(LightManager::GetInstance()->GetDirectionalLightEntity()->GetComponent(COMPONENT_LIGHT));
			glm::vec3 lightPos = -dirLight->Direction; // negative of the directional light's direction
			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 15.0f);
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
		}

		// render scene
		if (renderSystem != nullptr) {
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
			if (system->Name() && SYSTEM_RENDER) {
				renderSystem = dynamic_cast<SystemRender*>(system);
			}
			else if (system->Name() && SYSTEM_SHADOWMAP) {
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
