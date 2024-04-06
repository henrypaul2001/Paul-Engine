#include "Constraint.h"
namespace Engine {
	Constraint::Constraint(Entity& objectA, Entity& objectB) : objectA(objectA), objectB(objectB), active(true)
	{
		if (objectA.GetPhysicsComponent() == nullptr && objectB.GetPhysicsComponent() == nullptr) {
			throw std::invalid_argument("At least one constrained object must contain a physics component");
		}
	}
}