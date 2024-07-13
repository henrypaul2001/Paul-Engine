#include "ComponentStateController.h"
namespace Engine {
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