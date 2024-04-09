#pragma once
#include "Constraint.h"
namespace Engine {
	class ConstraintRotation : public Constraint
	{
	public:
		ConstraintRotation(Entity& objectA, Entity& objectB, glm::vec3 maxRotationOffset = glm::vec3(0.0f), bool controlXRotation = true, bool controlYRotation = true, bool controlZRotation = true, float bias = 0.000005f) : Constraint(objectA, objectB, bias) {
			this->controlXRotation = controlXRotation; 
			this->controlYRotation = controlYRotation;
			this->controlZRotation = controlZRotation;

			this->maxRotationOffset = maxRotationOffset;
		}

		~ConstraintRotation() {}

		void UpdateConstraint(float deltaTime) override;

		bool ControlXRotation() { return controlXRotation; }
		bool ControlYRotation() { return controlYRotation; }
		bool ControlZRotation() { return controlZRotation; }

		void ControlXRotation(bool control) { controlXRotation = control; }
		void ControlYRotation(bool control) { controlYRotation = control; }
		void ControlZRotation(bool control) { controlZRotation = control; }
	private:
		glm::vec3 maxRotationOffset;
		bool controlXRotation, controlYRotation, controlZRotation;
	};
}