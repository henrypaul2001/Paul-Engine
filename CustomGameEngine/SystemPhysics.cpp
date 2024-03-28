#include "SystemPhysics.h"
#include "Scene.h"
#include <algorithm>
#include <iostream>
#include "ComponentCollisionSphere.h"
namespace Engine 
{
	SystemPhysics::SystemPhysics()
	{
		gravity = 9.8f;
		gravityAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		airDensity = 1.225f;
	}

	SystemPhysics::~SystemPhysics()
	{

	}

	void SystemPhysics::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentPhysics* physics = nullptr;
			for (Component* c : components) {
				physics = dynamic_cast<ComponentPhysics*>(c);
				if (physics != nullptr) {
					break;
				}
			}

			Physics(transform, physics);
		}
	}

	void SystemPhysics::AfterAction()
	{

	}

	void SystemPhysics::Acceleration(ComponentTransform* transform, ComponentPhysics* physics)
	{
		float inverseMass = physics->InverseMass();
		glm::vec3 velocity = physics->Velocity();
		glm::vec3 acceleration = physics->Force() * inverseMass;

		// Apply gravity if component enables it and is not an immovable object
		if (physics->Gravity() && inverseMass > 0) {
			acceleration += glm::vec3(gravityAxis.x * -gravity, gravityAxis.y * -gravity, gravityAxis.z * -gravity) * Scene::dt;
		}

		float vMagnitude = glm::length(velocity); //m/s

		if (vMagnitude > 0) {
			float surfaceArea = physics->SurfaceArea(); // m2
			float coefficient = physics->DragCoefficient();
			float dragMagnitude = coefficient * airDensity * ((vMagnitude * vMagnitude) / 2.0f) * surfaceArea;
			glm::vec3 dragDirection = -glm::normalize(velocity);

			acceleration += (dragMagnitude * dragDirection) * Scene::dt;
		}

		velocity += acceleration * Scene::dt;
		physics->SetVelocity(velocity);

		// Angular
		physics->UpdateInertiaTensor(transform->GetOrientation());

		glm::vec3 angularAcceleration = physics->InverseInertiaTensor() * physics->Torque();
		glm::vec3 angularVelocity = physics->AngularVelocity();

		angularVelocity += angularAcceleration * Scene::dt;
		physics->SetAngularVelocity(angularVelocity);
	}

	void SystemPhysics::Physics(ComponentTransform* transform, ComponentPhysics* physics)
	{
		Acceleration(transform, physics);

		// Linear velocity
		glm::vec3 position = transform->Position();
		glm::vec3 velocity = physics->Velocity();
		transform->SetLastPosition(position);

		position += velocity * Scene::dt;

		transform->SetPosition(position + velocity);

		// Angular velocity
		glm::quat orientation = transform->GetOrientation();
		glm::vec3 angularVelocity = physics->AngularVelocity();

		orientation = orientation + (glm::quat(glm::vec3(angularVelocity * Scene::dt * 0.5f)) * orientation);
		orientation = glm::normalize(orientation);

		transform->SetOrientation(orientation);

		physics->ClearForces();
		physics->SetTorque(glm::vec3(0.0f));
	}
}