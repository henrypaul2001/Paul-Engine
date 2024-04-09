#pragma once
#include "Constraint.h"
namespace Engine {
    class ConstraintPosition : public Constraint
    {
    public:
        ConstraintPosition(Entity& objectA, Entity& objectB, float distance, float bias = 0.000005f, glm::vec3 relativeJointPositionA = glm::vec3(0.0f), glm::vec3 relativeJointPositionB = glm::vec3(0.0f)) : Constraint(objectA, objectB, bias) { this->distance = distance; this->relativeJointPositionA = relativeJointPositionA; this->relativeJointPositionB = relativeJointPositionB; }
        ~ConstraintPosition() {}

        void UpdateConstraint(float deltaTime) override;

        float Distance() { return distance; }
        void Distance(float newDistance) { this->distance = newDistance; }

        glm::vec3 RelativeJointPosA() { return relativeJointPositionA; }
        void RelativeJointPosA(glm::vec3 newPos) { this->relativeJointPositionA = newPos; }

        glm::vec3 RelativeJointPosB() { return relativeJointPositionB; }
        void RelativeJointPosB(glm::vec3 newPos) { this->relativeJointPositionB = newPos; }
    private:
        glm::vec3 relativeJointPositionA;
        glm::vec3 relativeJointPositionB;

        float distance;
    };
}