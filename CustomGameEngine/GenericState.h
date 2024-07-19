#pragma once
#include "State.h"
namespace Engine {
	typedef void (*StateFunc)(void*);

	class GenericState : public State
	{
	public:
		GenericState(const GenericState& old_state);

		GenericState(const std::string& name, StateFunc updateFunction, void* data, StateFunc enterFunction = nullptr, StateFunc exitFunction = nullptr) : State(name) {
			this->updateFunction = updateFunction;
			this->enterFunction = enterFunction;
			this->exitFunction = exitFunction;
			this->data = data;
		}
		
		~GenericState() {}
		
		void Update() override {
			if (data != nullptr) {
				if (updateFunction) {
					updateFunction(data);
				}
			}
		}

		void Enter() override {
			State::Enter();
			if (enterFunction) {
				enterFunction(data);
			}
		}

		void Exit() override {
			State::Exit();
			if (exitFunction) {
				exitFunction(data);
			}
		}

		// Copies to GenericState and it's functions, but points to the same original data
		State* Copy() override { return new GenericState(*this); }

		void SetData(void* data) { this->data = data; }

	protected:
		StateFunc updateFunction;
		StateFunc enterFunction;
		StateFunc exitFunction;
		void* data;
	};
}