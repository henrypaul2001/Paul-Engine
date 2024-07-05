#include "StateMachine.h"
namespace Engine {
	StateMachine::StateMachine()
	{
		activeState = nullptr;
	}

	StateMachine::~StateMachine()
	{

	}

	void StateMachine::AddState(State* newState)
	{
		states.emplace_back(newState);
		if (activeState == nullptr) {
			activeState = newState;
		}
	}

	void StateMachine::AddTransition(StateTransition* newTransition)
	{
		statesToTransitions.insert(std::make_pair(newTransition->GetSourceState(), newTransition));
	}

	void StateMachine::Update()
	{
		if (activeState) {
			// Update state
			activeState->Update();

			// Check transition conditions
			std::pair<TransitionIterator, TransitionIterator> range = statesToTransitions.equal_range(activeState);
			for (TransitionIterator& i = range.first; i != range.second; i++) {
				if (i->second->Condition()) {
					State* newState = i->second->GetDestinationState();
					activeState = newState;
				}
			}
		}
	}
}