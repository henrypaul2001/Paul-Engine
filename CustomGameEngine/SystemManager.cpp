#include "SystemManager.h"
namespace Engine
{
	SystemManager::SystemManager() {}

	SystemManager::~SystemManager()
	{
	}

	void SystemManager::ActionSystems(EntityManager* entityManager)
	{
		std::vector<Entity*> entityList = entityManager->Entities();
		for (System* s : systemList) {
			for (Entity* e : entityList) {
				s->OnAction(e);
			}
		}
	}

	void SystemManager::AddSystem(System* system)
	{
		System* result = FindSystem(system->Name());
		_ASSERT(result == nullptr, "System '" + system.Name() + "' already exists");
		systemList.push_back(system);
	}

	System* SystemManager::FindSystem(std::string name)
	{
		for (System* s : systemList) {
			if (s->Name() == name) {
				return s;
			}
		}

		return nullptr;
	}
}
