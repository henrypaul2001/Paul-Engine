#pragma once
#include "System.h"
namespace Engine {
	class SystemStateMachineUpdater : public System
	{
	public:
		SystemStateMachineUpdater() {}
		~SystemStateMachineUpdater() {}

		SystemTypes Name() override { return SYSTEM_STATE_UPDATER; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;

	private:
		const ComponentTypes MASK = (COMPONENT_STATE_CONTROLLER);
	};
}