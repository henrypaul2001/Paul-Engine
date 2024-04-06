#pragma once
#include "Entity.h"
namespace Engine {
	class Constraint
	{
	public:
		Constraint(Entity& objectA, Entity& objectB) { this->objectA = objectA; this->objectB = objectB; }
		~Constraint() {}

	protected:
		Entity& objectA;
		Entity& objectB;
	};
}