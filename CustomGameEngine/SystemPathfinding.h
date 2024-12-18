#pragma once
#include "System.h"
#include "ComponentPathfinder.h"
namespace Engine {
	class SystemPathfinding : public System
	{
    public:
        SystemPathfinding();
        ~SystemPathfinding();

        SystemTypes Name() override { return SYSTEM_PATHFINDING; }
        void Run(const std::vector<Entity*>& entityList) override;
        void OnAction(Entity* entity) override;
        void AfterAction() override;

    private:
        const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_PATHFINDER);
        void Update(ComponentTransform* transform, ComponentPathfinder* pathfinder);
	};
}