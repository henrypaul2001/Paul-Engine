#pragma once
#include "Component.h"
#include "StateMachine.h"
namespace Engine {
    class ComponentStateController : public Component
    {
    public:
        ComponentStateController(const ComponentStateController& old_component);
        ComponentStateController(const int maxStateHistorySize = 15);
        ~ComponentStateController();

        ComponentTypes ComponentType() override { return COMPONENT_STATE_CONTROLLER; }
        void Close() override;

        Component* Copy() override { return new ComponentStateController(*this); }

        StateMachine& GetStateMachine() { return *stateMachine; }

    private:
        StateMachine* stateMachine;
    };
}