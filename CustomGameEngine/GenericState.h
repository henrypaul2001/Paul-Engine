#pragma once
#include "State.h"
namespace Engine {
	typedef void (*StateFunc)(void*);

	class GenericState : public State
	{
	public:
		GenericState(const std::string& name, StateFunc updateFunction, void* data, StateFunc enterFunction = {}, StateFunc exitFunction = {}) : State(name) {
			this->updateFunction = updateFunction;
			this->enterFunction = enterFunction;
			this->exitFunction = exitFunction;
			this->data = data;
		}
		
		~GenericState() {}
		
		void Update() override {
			if (data != nullptr) {
				updateFunction(data);
			}
		}

		void Enter() override {
			State::Enter();
			enterFunction(data);
		}

		void Exit() override {
			State::Exit();
			exitFunction(data);
		}

	protected:
		StateFunc updateFunction;
		StateFunc enterFunction;
		StateFunc exitFunction;
		void* data;
	};
}