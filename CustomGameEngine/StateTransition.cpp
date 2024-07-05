#include "StateTransition.h"
#include "State.h"

namespace Engine {
	StateTransition::StateTransition(State* source, State* destination)
	{
		sourceState = source;
		destinationState = destination;
	}

	StateTransition::~StateTransition()
	{

	}
}