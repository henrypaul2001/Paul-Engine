#include "GenericState.h"
#include "StateMachine.h"
#include "ComponentStateController.h"
namespace Engine {
    GenericState::GenericState(const GenericState& old_state) : State(old_state.name)
    {
        this->updateFunction = old_state.updateFunction;
        this->enterFunction = old_state.enterFunction;
        this->exitFunction = old_state.exitFunction;
        this->data = old_state.data;
    }

    GenericState::GenericState(const std::string& name, StateFunc updateFunction, void* data, StateFunc enterFunction, StateFunc exitFunction) : State(name)
    {
        this->updateFunction = updateFunction;
        this->enterFunction = enterFunction;
        this->exitFunction = exitFunction;
        this->data = data;
    }

    void GenericState::Update()
    {
        if (data != nullptr) {
            if (updateFunction) {
                updateFunction(data);
            }
        }
        else {
            if (updateFunction) {
                updateFunction(parentStateMachine->GetParentComponent()->GetOwner());
            }
        }
    }

    void GenericState::Enter()
    {
        State::Enter();
        if (data != nullptr) {
            if (enterFunction) {
                enterFunction(data);
            }
        }
        else {
            if (enterFunction) {
                enterFunction(parentStateMachine->GetParentComponent()->GetOwner());
            }
        }
    }

    void GenericState::Exit()
    {
        State::Exit();
        if (data != nullptr) {
            if (exitFunction) {
                exitFunction(data);
            }
        }
        else {
            if (exitFunction) {
                exitFunction(parentStateMachine->GetParentComponent()->GetOwner());
            }
        }
    }
}