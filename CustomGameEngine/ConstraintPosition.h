#pragma once
#include "Constraint.h"
namespace Engine {
    class ConstraintPosition : public Constraint
    {
    public:
        ConstraintPosition(Entity& objectA, Entity& objectB, float distance) : Constraint(objectA, objectB) { this->distance = distance; }
        ~ConstraintPosition() {}

        void UpdateConstraint(float deltaTime) override;

        float Distance() { return distance; }
        float Distance(float newDistance) { this->distance = newDistance; }
    private:
        float distance;
    };
}
