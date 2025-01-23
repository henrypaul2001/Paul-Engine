#pragma once
#include "EntityManagerNew.h"
#include "ComponentPhysics.h"
namespace Engine {
	class Constraint
	{
	public:
		Constraint(EntityNew& objectA, EntityNew& objectB, const float bias) : objectA(objectA), objectB(objectB), active(true), bias(bias){}
		~Constraint() {}

		virtual void UpdateConstraint(EntityManagerNew* ecs, const float deltaTime) const = 0;

		float Bias() const { return bias; }
		void SetBias(const float newBias) { this->bias = newBias; }

		void Activate() { active = true; }
		void Deactivate() { active = false; }
		void SetActive(const bool active) { this->active = active; }
		bool IsActive() const { return active; }
	protected:
		float bias;
		bool active;
		EntityNew& objectA;
		EntityNew& objectB;
	};
}