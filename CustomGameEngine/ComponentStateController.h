#pragma once
#include "StateMachine.h"
namespace Engine {
    class ComponentStateController
    {
    public:
        ComponentStateController(const ComponentStateController& old_component);
        ComponentStateController(const int maxStateHistorySize = 15);
        ~ComponentStateController();

        StateMachine& GetStateMachine() { return *stateMachine; }

    private:
        StateMachine* stateMachine;
    };
}