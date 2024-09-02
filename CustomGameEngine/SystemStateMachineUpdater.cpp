#include "SystemStateMachineUpdater.h"
namespace Engine {
	void SystemStateMachineUpdater::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemStateMachineUpdater::Run");
		System::Run(entityList);
	}

	void SystemStateMachineUpdater::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			ComponentStateController* controller = entity->GetStateController();

			if (controller) {
				controller->GetStateMachine().Update();
			}
		}
	}

	void SystemStateMachineUpdater::AfterAction()
	{

	}
}