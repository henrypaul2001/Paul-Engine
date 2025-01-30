#pragma once
#include "System.h"
#include "ComponentTransform.h"
#include "ComponentPathfinder.h"
namespace Engine {
	class SystemPathfinding : public System
	{
    public:
        SystemPathfinding(EntityManager* ecs) : System(ecs) {}
        ~SystemPathfinding() {}

        constexpr const char* SystemName() override { return "SYSTEM_PATHFINDING"; }

        void OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentPathfinder& pathfinder);
        void AfterAction();
	};
}