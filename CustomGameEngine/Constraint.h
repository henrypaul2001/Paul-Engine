#pragma once
#include "EntityManagerNew.h"
#include "ComponentPhysics.h"
namespace Engine {
	class Constraint
	{
	public:
		Constraint(unsigned int entityIDA, unsigned int entityIDB, const float bias) : entityIDA(entityIDA), entityIDB(entityIDB), active(true), bias(bias){}
		~Constraint() {}

		virtual void UpdateConstraint(EntityManagerNew& ecs, const float deltaTime) const = 0;

		float Bias() const { return bias; }
		void SetBias(const float newBias) { this->bias = newBias; }

		void Activate() { active = true; }
		void Deactivate() { active = false; }
		void SetActive(const bool active) { this->active = active; }
		bool IsActive() const { return active; }
	protected:
		float bias;
		bool active;
		unsigned int entityIDA;
		unsigned int entityIDB;
	};
}