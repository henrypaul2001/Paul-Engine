#include "SystemManager.h"
namespace Engine
{
	SystemManager::SystemManager() {}

	SystemManager::~SystemManager()
	{
		delete& systemList;
	}

	void SystemManager::ActionSystems(EntityManager& entityManager)
	{
		
	}

	void SystemManager::AddSystem(System& system)
	{
		System* result = FindSystem(system.Name());
		_ASSERT(result != nullptr, "System '" + system.Name() + "' already exists");
		systemList.push_back(system);
	}

	System* SystemManager::FindSystem(std::string name)
	{
		for (System& s : systemList) {
			if (s.Name() == name) {
				return &s;
			}
		}

		return nullptr;
	}
}
