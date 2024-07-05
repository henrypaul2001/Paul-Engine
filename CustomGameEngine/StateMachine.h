#pragma once
#include <map>
#include "State.h"
#include <vector>
#include "StateTransition.h"
namespace Engine {

	typedef std::multimap<State*, StateTransition*>::iterator TransitionIterator;

	class StateMachine
	{
	public:
		StateMachine();
		~StateMachine();

		void AddState(State* newState);
		void AddTransition(StateTransition* newTransition);
		
		void Update();
	private:
		State* activeState;
		std::vector<State*> states;
		std::multimap<State*, StateTransition*> statesToTransitions;
	};
}