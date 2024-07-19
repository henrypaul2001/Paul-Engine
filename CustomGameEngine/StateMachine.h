#pragma once
#include <map>
#include "State.h"
#include <vector>
#include <queue>
#include "StateTransition.h"
namespace Engine {

	typedef std::multimap<State*, StateTransition*>::iterator TransitionIterator;

	class StateMachine
	{
	public:
		StateMachine(const StateMachine& old_stateMachine);
		StateMachine(const int maxHistorySize = 15);
		~StateMachine();

		void AddState(State* newState);
		void AddTransition(StateTransition* newTransition);
		
		void Update();

		const std::queue<State*>& GetStateHistory() const { return stateHistory; }

		const std::string& GetActiveStateName() const { return activeState->GetName(); }
	
		std::vector<State*> GetStates() const { return states; }
		std::vector<StateTransition*> GetTransitions() const { return transitions; }

	private:
		State* activeState;
		std::vector<State*> states;
		std::vector<StateTransition*> transitions;
		std::multimap<State*, StateTransition*> statesToTransitions;
		std::queue<State*> stateHistory;
		int maxHistorySize;
	};
}