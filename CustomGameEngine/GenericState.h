#pragma once
#include "State.h"
namespace Engine {
	typedef void (*StateFunc)(void*);

	class GenericState : public State
	{
	public:
		GenericState(const std::string& name, StateFunc function, void* data) : State(name) {
			this->function = function;
			this->data = data;
		}
		
		~GenericState() {}
		
		void Update() override {
			if (data != nullptr) {
				function(data);
			}
		}

		void Enter() override {
			State::Enter();
		}

		void Exit() override {
			State::Exit();
		}

	protected:
		StateFunc function;
		void* data;
	};
}