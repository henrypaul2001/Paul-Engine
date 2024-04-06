#pragma once
#include "Constraint.h"
namespace Engine {
    class ConstraintPosition : public Constraint
    {
    public:
        ConstraintPosition(Entity& objectA, Entity& objectB, float distance, float bias = 0.0f) : Constraint(objectA, objectB) { this->distance = distance; this->bias = bias; }
        ~ConstraintPosition() {}

        void UpdateConstraint(float deltaTime) override;

        float Distance() { return distance; }
        float Distance(float newDistance) { this->distance = newDistance; }

        float Bias() { return bias; }
        float Bias(float newBias) { this->bias = newBias; }
    private:
        float distance;
        float bias;
    };
}
