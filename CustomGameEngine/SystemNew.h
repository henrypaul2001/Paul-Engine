#pragma once
#include "EntityManagerNew.h"

namespace Engine {
	class SystemNew {
	public:
		SystemNew(EntityManagerNew* ecs) : active_ecs(ecs) {}

		virtual constexpr const char* SystemName() = 0;
	protected:
		EntityManagerNew* active_ecs;
	};
}