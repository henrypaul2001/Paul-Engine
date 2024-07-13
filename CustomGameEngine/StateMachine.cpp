#include "StateMachine.h"
namespace Engine {
	StateMachine::StateMachine(const int maxHistorySize)
	{
		this->maxHistorySize = maxHistorySize;
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