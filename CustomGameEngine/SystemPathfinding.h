#pragma once
#include "SystemNew.h"
#include "ComponentTransform.h"
#include "ComponentPathfinder.h"
namespace Engine {
	class SystemPathfinding : public SystemNew
	{
    public:
        SystemPathfinding(EntityManagerNew* ecs) : SystemNew(ecs) {}
        ~SystemPathfinding() {}

        constexpr const char* SystemName() override { return "SYSTEM_PATHFINDING"; }

        void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentPathfinder& pathfinder);
        void AfterAction();
	};
}