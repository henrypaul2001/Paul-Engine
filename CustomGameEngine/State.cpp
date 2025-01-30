#include "State.h"
#include "StateMachine.h"
#include "ComponentStateController.h"
namespace Engine {
    void State::Enter(EntityManager* ecs, const unsigned int entityID)
    {
        std::cout << "Enter " << name << " state" << std::endl;
    }

    void State::Exit(EntityManager* ecs, const unsigned int entityID)
    {
        std::cout << "Exit " << name << " state" << std::endl;
    }
}