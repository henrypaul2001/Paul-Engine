#pragma once
#include "State.h"
namespace Engine {
	typedef void (*StateFunc)(void*);

	class GenericState : public State
	{
	public:
		GenericState(StateFunc function, void* data) {
			this->function = function;
			this->data = data;
		}
		
		~GenericState() {}
		
		void Update() override {
			if (data != nullptr) {
				function(data);
			}
		}
	protected:
		StateFunc function;
		void* data;
	};
}