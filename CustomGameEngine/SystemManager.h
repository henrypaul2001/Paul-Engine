#pragma once
#include <vector>
#include "System.h"
#include "EntityManager.h"
#include <string>
#include "SystemRender.h"
#include "SystemShadowMapping.h"
#include "SystemReflectionBaking.h"
#include "CollisionResolver.h"
#include "ConstraintSolver.h"
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

		SystemRender* renderSystem;
		SystemShadowMapping* shadowmapSystem;
		SystemReflectionBaking* reflectionProbeSystem;
		CollisionResolver* collisionResponseSystem;
		ConstraintSolver* constraintSolver;
	public:
		SystemManager();
		~SystemManager();

		System* FindSystem(SystemTypes name, SystemLists list);
		void ActionUpdateSystems(EntityManager* entityManager);
		void ActionRenderSystems(EntityManager* entityManager, int SCR_WIDTH, int SCR_HEIGHT);
		void AddSystem(System* system, SystemLists list);
		void AddCollisionResponseSystem(CollisionResolver* collisionResponder) { this->collisionResponseSystem = collisionResponder; }
		void AddConstraintSolver(ConstraintSolver* constraintSolver) { this->constraintSolver = constraintSolver; }

		void BakeReflectionProbes(const std::vector<Entity*>& entities, const bool discardUnfilteredCapture = true);
	};
}

