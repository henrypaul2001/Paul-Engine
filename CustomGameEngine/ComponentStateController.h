#pragma once
#include "Component.h"
#include "StateMachine.h"
namespace Engine {
    class ComponentStateController : public Component
    {
    public:
        ComponentStateController(const int maxStateHistorySize = 15);
        ~ComponentStateController();

        ComponentTypes ComponentType() override { return COMPONENT_STATE_CONTROLLER; }
        void Close() override;

        StateMachine& GetStateMachine() { return *stateMachine; }

    private:
        StateMachine* stateMachine;
    };
}