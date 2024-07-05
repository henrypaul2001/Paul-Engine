#pragma once
#include "State.h"
namespace Engine {
	class StateTransition
	{
	public:
		StateTransition(State* source, State* destination);
		~StateTransition();

		virtual bool Condition() const = 0;

		State* GetDestinationState() const { return destinationState; }
		State* GetSourceState() const { return sourceState; }

	protected:
		State* sourceState;
		State* destinationState;
	};
}