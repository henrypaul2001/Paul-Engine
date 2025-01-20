#pragma once
#include "SystemNew.h"
#include "ComponentStateController.h"
namespace Engine {
	class SystemStateMachineUpdater : public SystemNew
	{
	public:
		SystemStateMachineUpdater(EntityManagerNew* ecs) : SystemNew(ecs) {}
		~SystemStateMachineUpdater() {}

		constexpr const char* SystemName() override { return "SYSTEM_STATE_UPDATER"; }

		void OnAction(const unsigned int entityID, ComponentStateController& controller);
		void AfterAction();
	};
}