#pragma once
#include <vector>
#include "System.h"
#include "EntityManager.h"
#include <string>
namespace Engine
{
	class SystemManager
	{
	private:
		std::vector<System*> systemList;
		System* FindSystem(std::string name);

	public:
		SystemManager();
		~SystemManager();

		void ActionSystems(EntityManager* entityManager);
		void AddSystem(System* system);
	};
}

