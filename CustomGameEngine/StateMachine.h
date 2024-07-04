#pragma once
#include <map>
namespace Engine {
	class StateMachine
	{
	public:
		StateMachine();
		~StateMachine();

		//void AddState(State* newState);
		//void AddTransition(StateTransition* newTransition);
		
		void Update();
	private:
		//State* activeState;
		//std::vector<State*> states;
		//std::multimap<State*, StateTransition*> statesToTransitions;
	};
}