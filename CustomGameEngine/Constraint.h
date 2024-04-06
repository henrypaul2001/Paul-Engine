#pragma once
#include "Entity.h"
namespace Engine {
	class Constraint
	{
	public:
		Constraint(Entity& objectA, Entity& objectB);
		~Constraint() {}

		virtual void UpdateConstraint(float deltaTime) = 0;

		void Activate() { active = true; }
		void Deactivate() { active = false; }
		bool IsActive() { return active; }
	protected:
		bool active;
		Entity& objectA;
		Entity& objectB;
	};
}