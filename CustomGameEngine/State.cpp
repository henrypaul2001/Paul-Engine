#include "State.h"
#include "StateMachine.h"
#include "ComponentStateController.h"
namespace Engine {
    void State::Enter()
    {
        std::cout << "Enter " << name << " state" << std::endl;
    }

    void State::Exit()
    {
        std::cout << "Exit " << name << " state" << std::endl;
    }
}