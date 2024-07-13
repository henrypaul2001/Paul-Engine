#include "SystemStateMachineUpdater.h"
namespace Engine {
	void SystemStateMachineUpdater::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentStateController* controller = nullptr;
			for (Component* c : components) {
				controller = dynamic_cast<ComponentStateController*>(c);
				if (controller != nullptr) {
					break;
				}
			}

			if (controller) {
				controller->GetStateMachine().Update();
			}
		}
	}

	void SystemStateMachineUpdater::AfterAction()
	{

	}
}