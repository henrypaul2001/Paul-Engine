#pragma once
#include "EntityManagerNew.h"

namespace Engine {
	class System {
	public:
		System(EntityManagerNew* ecs) : active_ecs(ecs) {}

		virtual constexpr const char* SystemName() = 0;
	protected:
		EntityManagerNew* active_ecs;
	};
}