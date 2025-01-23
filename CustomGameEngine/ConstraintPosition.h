#pragma once
#include "Constraint.h"
namespace Engine {
    class ConstraintPosition : public Constraint
    {
    public:
        ConstraintPosition(EntityNew& objectA, EntityNew& objectB, const float distance, const float bias = 0.000005f, const glm::vec3& relativeJointPositionA = glm::vec3(0.0f), const glm::vec3& relativeJointPositionB = glm::vec3(0.0f)) : Constraint(objectA, objectB, bias), distance(distance), relativeJointPositionA(relativeJointPositionA), relativeJointPositionB(relativeJointPositionB) {}
        ~ConstraintPosition() {}

        void UpdateConstraint(EntityManagerNew& ecs, const float deltaTime) const override;

        float Distance() const { return distance; }
        void SetDistance(const float newDistance) { this->distance = newDistance; }

        const glm::vec3& RelativeJointPosA() const { return relativeJointPositionA; }
        void SetRelativeJointPosA(const glm::vec3& newPos) { this->relativeJointPositionA = newPos; }

        const glm::vec3& RelativeJointPosB() const { return relativeJointPositionB; }
        void RelativeJointPosB(const glm::vec3& newPos) { this->relativeJointPositionB = newPos; }
    private:
        glm::vec3 relativeJointPositionA;
        glm::vec3 relativeJointPositionB;

        float distance;
    };
}