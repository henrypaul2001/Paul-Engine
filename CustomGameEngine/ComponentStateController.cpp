#include "ComponentStateController.h"
namespace Engine {
	ComponentStateController::ComponentStateController(const ComponentStateController& old_component)
	{
		this->stateMachine = new StateMachine(*old_component.stateMachine);
		stateMachine->SetParentComponent(this);
	}

	ComponentStateController::ComponentStateController(const int maxStateHistorySize)
	{
		stateMachine = new StateMachine(maxStateHistorySize);
		stateMachine->SetParentComponent(this);
	}

	ComponentStateController::~ComponentStateController() { delete stateMachine; }
}