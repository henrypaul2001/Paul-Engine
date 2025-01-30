#pragma once
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/quaternion.hpp>
namespace Engine {
    class ComponentPhysics
    {
    public:
        ComponentPhysics(const float mass = 10.0f, const float drag = 1.05f, const float surfaceArea = 1.0f, const float elasticity = 0.5f, const bool gravity = true, const bool cuboidInertiaTensor = false);
        ~ComponentPhysics();

        // Get

        const glm::vec3& Velocity() const                       { return velocity; }
        const glm::vec3& Force() const                          { return force; }
        const float InverseMass() const                         { return inverseMass; }
        const float RawMass() const                             { return mass; }
        const bool Gravity() const                              { return gravity; }
        const glm::mat3& InertiaTensor() const                  { return inertiaTensor; }
        const glm::vec3& Torque() const                         { return torque; }
        const glm::vec3& AngularVelocity() const                { return angularVelocity; }
        const glm::mat3& InverseInertiaTensor() const           { return inverseInertiaTensor; }
        const glm::vec3& InverseInertia() const                 { return inverseInertia; }
        const float DragCoefficient() const                     { return dragCoefficient; }
        const float SurfaceArea() const                         { return surfaceArea; }
        const float Elasticity() const                          { return elasticity; }

        // Set

        void SetMass(const float newMass)                       { mass = newMass; UpdateInverseMass(); }
        void SetVelocity(const glm::vec3& newVelocity)          { this->velocity = newVelocity; }
        void SetAngularVelocity(const glm::vec3& newVelocity)   { this->angularVelocity = newVelocity; }
        void SetTorque(const glm::vec3& newTorque)              { torque = newTorque; }
        void AddTorque(const glm::vec3& newTorque)              { torque += newTorque; }
        void SetGravity(const bool newGravity)                  { gravity = newGravity; }
        void SetDragCoefficient(const float newDrag)            { dragCoefficient = newDrag; }
        void SetSurfaceArea(const float surfaceArea)            { this->surfaceArea = surfaceArea; }
        void SetElasticity(const float elasticity)              { this->elasticity = elasticity; }

        // Update

        void ClearForces() { force = glm::vec3(0.0f); }
        void AddForce(const glm::vec3& force) { this->force += force; }
        void AddForce(const glm::vec3& force, const glm::vec3& forcePositionLocal) {
            this->force += force;
            torque += glm::cross(forcePositionLocal, force);
        }

        void ApplyLinearImpulse(const glm::vec3& force) { velocity += force * inverseMass; }
        void ApplyAngularImpulse(const glm::vec3 angularForce) { angularVelocity += inverseInertiaTensor * angularForce; }

        void UpdateInertiaTensor(const glm::quat& orientation);

    private:
        void UpdateInverseMass() { inverseMass = 1.0f / mass; }

        bool gravity;

        float surfaceArea;
        float dragCoefficient;
        float inverseMass;
        float mass;
        float elasticity;

        glm::vec3 velocity;
        glm::vec3 angularVelocity;
        glm::vec3 torque;
        glm::mat3 inertiaTensor;

        glm::mat3 inverseInertiaTensor;
        glm::vec3 inverseInertia;

        glm::vec3 force;
    };
}