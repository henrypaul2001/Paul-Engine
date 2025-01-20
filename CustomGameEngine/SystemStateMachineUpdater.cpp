#include "SystemStateMachineUpdater.h"
namespace Engine {
	void SystemStateMachineUpdater::OnAction(const unsigned int entityID, ComponentStateController& controller) { controller.GetStateMachine().Update(); }
	void SystemStateMachineUpdater::AfterAction() {}
}