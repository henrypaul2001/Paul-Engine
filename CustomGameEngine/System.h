#pragma once
#include "EntityManager.h"

namespace Engine {
	class System {
	public:
		System(EntityManager* ecs) : active_ecs(ecs) {}

		virtual constexpr const char* SystemName() = 0;
	protected:
		EntityManager* active_ecs;
	};
}