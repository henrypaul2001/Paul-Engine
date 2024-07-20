#pragma once
#include "State.h"
namespace Engine {
	typedef void (*StateFunc)(void*);

	class GenericState : public State
	{
	public:
		GenericState(const GenericState& old_state);

		GenericState(const std::string& name, StateFunc updateFunction, void* data, StateFunc enterFunction = nullptr, StateFunc exitFunction = nullptr);
		
		~GenericState() {}
		
		void Update() override;

		void Enter() override;

		void Exit() override;

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