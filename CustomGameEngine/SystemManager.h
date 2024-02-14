#pragma once
#include <vector>
#include "System.h"
#include "EntityManager.h"
#include <string>
#include "SystemRender.h"
namespace Engine
{
	enum SystemLists {
		RENDER_SYSTEMS = 0,
		UPDATE_SYSTEMS = 1 << 0
	};

	class SystemManager
	{
	private:
		std::vector<System*> updateSystemList;
		std::vector<System*> renderSystemList;

	public:
		SystemManager();
		~SystemManager();

		System* FindSystem(SystemTypes name, SystemLists list);
		void ActionUpdateSystems(EntityManager* entityManager);
		void ActionRenderSystems(EntityManager* entityManager);
		void AddSystem(System* system, SystemLists list);
	};
}

