#pragma once
#include "System.h"
#include "ComponentStateController.h"
namespace Engine {
	class SystemStateMachineUpdater : public System
	{
	public:
		SystemStateMachineUpdater(EntityManagerNew* ecs) : System(ecs) {}
		~SystemStateMachineUpdater() {}

		constexpr const char* SystemName() override { return "SYSTEM_STATE_UPDATER"; }

		void OnAction(const unsigned int entityID, ComponentStateController& controller);
		void AfterAction();
	};
}