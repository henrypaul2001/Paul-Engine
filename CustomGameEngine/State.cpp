#include "State.h"
#include "StateMachine.h"
#include "ComponentStateController.h"
namespace Engine {
    void State::Enter(EntityManagerNew* ecs, const unsigned int entityID)
    {
        std::cout << "Enter " << name << " state" << std::endl;
    }

    void State::Exit(EntityManagerNew* ecs, const unsigned int entityID)
    {
        std::cout << "Exit " << name << " state" << std::endl;
    }
}