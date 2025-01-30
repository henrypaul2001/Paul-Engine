#include "SystemStateMachineUpdater.h"
#include "ScopeTimer.h"
namespace Engine {
	void SystemStateMachineUpdater::OnAction(const unsigned int entityID, ComponentStateController& controller) {
		SCOPE_TIMER("SystemStateMachineUpdater::OnAction()");
		controller.GetStateMachine().Update(active_ecs, entityID);
	}
	void SystemStateMachineUpdater::AfterAction() {}
}