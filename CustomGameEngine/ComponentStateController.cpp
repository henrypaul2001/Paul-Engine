#include "ComponentStateController.h"
namespace Engine {
	ComponentStateController::ComponentStateController(const ComponentStateController& old_component)
	{
		this->owner = nullptr;

		this->stateMachine = new StateMachine(*old_component.stateMachine);
		stateMachine->SetParentComponent(this);
	}

	ComponentStateController::ComponentStateController(const int maxStateHistorySize)
	{
		stateMachine = new StateMachine(maxStateHistorySize);
		stateMachine->SetParentComponent(this);
	}

	ComponentStateController::~ComponentStateController()
	{
		delete stateMachine;
	}

	void ComponentStateController::Close()
	{

	}
}