#include "ComponentStateController.h"
namespace Engine {
	ComponentStateController::ComponentStateController(const ComponentStateController& old_component)
	{
		this->owner = nullptr;

		this->stateMachine = new StateMachine(*old_component.stateMachine);
	}

	ComponentStateController::ComponentStateController(const int maxStateHistorySize)
	{
		stateMachine = new StateMachine(maxStateHistorySize);
	}

	ComponentStateController::~ComponentStateController()
	{
		delete stateMachine;
	}

	void ComponentStateController::Close()
	{

	}
}