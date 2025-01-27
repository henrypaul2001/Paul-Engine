#pragma once
#include "State.h"
namespace Engine {
	class StateTransition
	{
	public:
		StateTransition(State* source, State* destination);
		~StateTransition();

		virtual StateTransition* Copy() = 0;

		virtual bool Condition(EntityManagerNew* ecs, const unsigned int entityID) = 0;

		State* GetDestinationState() const { return destinationState; }
		State* GetSourceState() const { return sourceState; }

		void SetSourceState(State* newSource) { this->sourceState = newSource; }
		void SetDestinationState(State* newDestination) { this->destinationState = newDestination; }

	protected:
		State* sourceState;
		State* destinationState;
	};
}