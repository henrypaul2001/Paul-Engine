#include "StateMachine.h"
namespace Engine {
	StateMachine::StateMachine(const StateMachine& old_stateMachine)
	{
		this->maxHistorySize = old_stateMachine.maxHistorySize;
		
		// Copy states

		// Copy transitions
		//		based on state indexes
	}

	StateMachine::StateMachine(const int maxHistorySize)
	{
		this->maxHistorySize = maxHistorySize;
		activeState = nullptr;
	}

	StateMachine::~StateMachine()
	{
		// delete states and transitions
		for (State* s : states) {
			std::pair<TransitionIterator, TransitionIterator> range = statesToTransitions.equal_range(s);
			for (TransitionIterator& i = range.first; i != range.second; i++) {
				delete i->second;
			}
			delete s;
		}
	}

	void StateMachine::AddState(State* newState)
	{
		states.emplace_back(newState);
		if (activeState == nullptr) {
			activeState = newState;
			activeState->Enter();
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
					activeState->Exit();
					State* newState = i->second->GetDestinationState();
					stateHistory.push(activeState);
					if (stateHistory.size() > maxHistorySize) {
						stateHistory.pop();
					}
					activeState = newState;
					activeState->Enter();
				}
			}
		}
	}
}