#include "SystemStateMachineUpdater.h"
namespace Engine {
	void SystemStateMachineUpdater::OnAction(const unsigned int entityID, ComponentStateController& controller) { controller.GetStateMachine().Update(active_ecs, entityID); }
	void SystemStateMachineUpdater::AfterAction() {}
}