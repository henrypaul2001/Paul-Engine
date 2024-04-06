#pragma once
#include "Entity.h"
namespace Engine {
	class Constraint
	{
	public:
		Constraint(Entity& objectA, Entity& objectB) { this->objectA = objectA; this->objectB = objectB; active = true; }
		~Constraint() {}

		void Activate() { active = true; }
		void Deactivate() { active = false; }
		bool IsActive() { return active; }
	protected:
		bool active;
		Entity& objectA;
		Entity& objectB;
	};
}