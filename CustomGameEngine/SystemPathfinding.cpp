#include "SystemPathfinding.h"
namespace Engine {
	SystemPathfinding::SystemPathfinding()
	{

	}

	SystemPathfinding::~SystemPathfinding()
	{

	}

	void SystemPathfinding::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentPathfinder* pathfinder = nullptr;
			for (Component* c : components) {
				pathfinder = dynamic_cast<ComponentPathfinder*>(c);
				if (pathfinder != nullptr) {
					break;
				}
			}

			Update(transform, pathfinder);
		}
	}

	void SystemPathfinding::AfterAction()
	{

	}

	void SystemPathfinding::Update(ComponentTransform* transform, ComponentPathfinder* pathfinder)
	{
		// Check if entity has reached next position

			// if false
			
			// Check if entity can be moved (is grounded, isn't stunned, etc)
				// Move towards next position at move speed

			// if true
			// Pop waypoint stack and set next waypoint


	}
}