#include "SystemManager.h"
namespace Engine
{
	SystemManager::SystemManager() 
	{

	}

	SystemManager::~SystemManager()
	{

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

	void SystemManager::ActionRenderSystems(EntityManager* entityManager)
	{
		std::vector<Entity*> entityList = entityManager->Entities();
		for (System* s : renderSystemList) {
			for (Entity* e : entityList) {
				s->OnAction(e);
			}
			s->AfterAction();
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
