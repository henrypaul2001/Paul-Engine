#pragma once
#include "Constraint.h"
namespace Engine {
	class ConstraintRotation : public Constraint
	{
	public:
		ConstraintRotation(unsigned int entityIDA, unsigned int entityIDB, const glm::vec3& maxRotationOffset = glm::vec3(0.0f), const bool controlXRotation = true, const bool controlYRotation = true, const bool controlZRotation = true, const float bias = 0.000005f) : Constraint(entityIDA, entityIDB, bias),
			controlXRotation(controlXRotation), controlYRotation(controlYRotation), controlZRotation(controlZRotation), maxRotationOffset(maxRotationOffset) {}
		~ConstraintRotation() {}

		void UpdateConstraint(EntityManager& ecs, const float deltaTime) const override;

		bool ControlXRotation() const { return controlXRotation; }
		bool ControlYRotation() const { return controlYRotation; }
		bool ControlZRotation() const { return controlZRotation; }

		void SetControlXRotation(const bool control) { controlXRotation = control; }
		void SetControlYRotation(const bool control) { controlYRotation = control; }
		void SetControlZRotation(const bool control) { controlZRotation = control; }
	private:
		glm::vec3 maxRotationOffset;
		bool controlXRotation, controlYRotation, controlZRotation;
	};
}