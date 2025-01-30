#include "StateMachine.h"
#include <unordered_map>
#include "ComponentStateController.h"
namespace Engine {
	StateMachine::StateMachine(const StateMachine& old_stateMachine)
	{
		this->maxHistorySize = old_stateMachine.maxHistorySize;

		std::unordered_map<State*, State*> stateMap; // <original state, copied state>

		std::vector<State*> originalStates = old_stateMachine.states;
		
		// Copy states
		for (State* s : originalStates) {
			stateMap[s] = s->Copy();
			stateMap[s]->SetParentStateMachine(this);
			this->AddState(stateMap[s]);
		}
		this->activeState = stateMap[old_stateMachine.activeState];

		// Copy transitions
		std::vector<StateTransition*> originalTransitions = old_stateMachine.transitions;
		for (StateTransition* t : originalTransitions) {
			StateTransition* newTransition = t->Copy();

			newTransition->SetSourceState(stateMap[t->GetSourceState()]);
			newTransition->SetDestinationState(stateMap[t->GetDestinationState()]);

			this->AddTransition(newTransition);
		}

		// Copy state history
		std::queue<State*> oldHistory = old_stateMachine.stateHistory;
		while (!oldHistory.empty()) {
			State* s = oldHistory.front();
			this->stateHistory.push(stateMap[s]);
			oldHistory.pop();
		}

		firstIteration = old_stateMachine.firstIteration;
	}

	StateMachine::StateMachine(const int maxHistorySize)
	{
		this->maxHistorySize = maxHistorySize;
		activeState = nullptr;
		firstIteration = true;
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
		newState->SetParentStateMachine(this);
		if (activeState == nullptr) {
			activeState = newState;
		}
	}

	void StateMachine::AddTransition(StateTransition* newTransition)
	{
		statesToTransitions.insert(std::make_pair(newTransition->GetSourceState(), newTransition));
		transitions.push_back(newTransition);
	}

	void StateMachine::Update(EntityManager* ecs, const unsigned int entityID)
	{
		if (activeState) {

			if (firstIteration) {
				firstIteration = false;
				activeState->Enter(ecs, entityID);
			}

			// Update state
			activeState->Update(ecs, entityID);

			// Check transition conditions
			std::pair<TransitionIterator, TransitionIterator> range = statesToTransitions.equal_range(activeState);
			for (TransitionIterator& i = range.first; i != range.second; i++) {
				if (i->second->Condition(ecs, entityID)) {
					activeState->Exit(ecs, entityID);
					State* newState = i->second->GetDestinationState();
					stateHistory.push(activeState);
					if (stateHistory.size() > maxHistorySize) {
						stateHistory.pop();
					}
					activeState = newState;
					activeState->Enter(ecs, entityID);
				}
			}
		}
	}
}