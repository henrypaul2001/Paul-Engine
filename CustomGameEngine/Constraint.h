#pragma once
#include "Entity.h"
namespace Engine {
	class Constraint
	{
	public:
		Constraint(Entity& objectA, Entity& objectB, float bias = 0.000005f);
		~Constraint() {}

		virtual void UpdateConstraint(float deltaTime) = 0;

		float Bias() { return bias; }
		void Bias(float newBias) { this->bias = newBias; }

		void Activate() { active = true; }
		void Deactivate() { active = false; }
		bool IsActive() { return active; }
	protected:
		float bias;
		bool active;
		Entity& objectA;
		Entity& objectB;
	};
}