#pragma once
#include "Constraint.h"
namespace Engine {
    class ConstraintPosition : public Constraint
    {
    public:
        ConstraintPosition(Entity& objectA, Entity& objectB, float distance, float bias = 0.0f, glm::vec3 relativeJointPositionA = glm::vec3(0.0f), glm::vec3 relativeJointPositionB = glm::vec3(0.0f)) : Constraint(objectA, objectB) { this->distance = distance; this->bias = bias; this->relativeJointPositionA = relativeJointPositionA; this->relativeJointPositionB = relativeJointPositionB; }
        ~ConstraintPosition() {}

        void UpdateConstraint(float deltaTime) override;

        float Distance() { return distance; }
        void Distance(float newDistance) { this->distance = newDistance; }

        float Bias() { return bias; }
        void Bias(float newBias) { this->bias = newBias; }

        glm::vec3 RelativeJointPosA() { return relativeJointPositionA; }
        void RelativeJointPosA(glm::vec3 newPos) { this->relativeJointPositionA = newPos; }

        glm::vec3 RelativeJointPosB() { return relativeJointPositionB; }
        void RelativeJointPosB(glm::vec3 newPos) { this->relativeJointPositionB = newPos; }
    private:
        glm::vec3 relativeJointPositionA;
        glm::vec3 relativeJointPositionB;

        float distance;
        float bias;
    };
}