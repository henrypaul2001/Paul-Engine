#include "GenericState.h"
namespace Engine {
    GenericState::GenericState(const GenericState& old_state) : State(old_state.name)
    {
        this->updateFunction = old_state.updateFunction;
        this->enterFunction = old_state.enterFunction;
        this->exitFunction = old_state.exitFunction;
        this->data = old_state.data;
    }
}