#include "SystemManager.h"
#include "RenderManager.h"
#include "LightManager.h"
#include "ComponentLight.h"
#include "ResourceManager.h"
#include "ForwardPipeline.h"
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
			if (s->Name() == SYSTEM_PHYSICS) {
				// Collision response
				collisionResponseSystem->OnAction();
				collisionResponseSystem->AfterAction();
			}

			for (Entity* e : entityList) {
				s->OnAction(e);
			}
			s->AfterAction();
		}
	}

	void SystemManager::ActionRenderSystems(EntityManager* entityManager, int SCR_WIDTH, int SCR_HEIGHT)
	{
		RenderManager::GetInstance()->RunRenderPipeline(renderSystemList, entityManager->Entities());
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
