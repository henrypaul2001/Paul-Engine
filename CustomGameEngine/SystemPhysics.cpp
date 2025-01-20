#include "SystemPhysics.h"
#include "Scene.h"
namespace Engine 
{
	void SystemPhysics::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentPhysics& physics)
	{
		Acceleration(transform, physics);

		// Linear velocity
		glm::vec3 position = transform.Position();
		const glm::vec3& velocity = physics.Velocity();
		//transform->SetLastPosition(position);

		position += velocity * Scene::dt;

		transform.SetPosition(position + velocity); // this is bizarre, why are we adding velocity twice? todo:

		// Angular velocity
		glm::quat orientation = transform.GetOrientation();
		const glm::vec3& angularVelocity = physics.AngularVelocity();

		orientation = orientation + (glm::quat(glm::vec3(angularVelocity * Scene::dt * 0.5f)) * orientation);
		orientation = glm::normalize(orientation);

		transform.SetOrientation(orientation);

		physics.ClearForces();
		physics.SetTorque(glm::vec3(0.0f));
	}

	void SystemPhysics::Acceleration(ComponentTransform& transform, ComponentPhysics& physics)
	{
		const float inverseMass = physics.InverseMass();
		glm::vec3 velocity = physics.Velocity();
		glm::vec3 acceleration = physics.Force() * inverseMass;

		// Apply gravity if component enables it and is not an immovable object
		if (physics.Gravity() && inverseMass > 0) {
			acceleration += glm::vec3(gravityAxis.x * -gravity, gravityAxis.y * -gravity, gravityAxis.z * -gravity) * Scene::dt;
		}

		const float vMagnitude = glm::length(velocity); //m/s

		if (vMagnitude > 0) {
			const float surfaceArea = physics.SurfaceArea(); // m2
			const float coefficient = physics.DragCoefficient();
			const float dragMagnitude = coefficient * airDensity * ((vMagnitude * vMagnitude) / 2.0f) * surfaceArea;
			const glm::vec3 dragDirection = -glm::normalize(velocity);

			acceleration += (dragMagnitude * dragDirection) * Scene::dt;
		}

		velocity += acceleration * Scene::dt;
		physics.SetVelocity(velocity);

		// Angular
		physics.UpdateInertiaTensor(transform.GetOrientation());

		const glm::vec3 angularAcceleration = physics.InverseInertiaTensor() * physics.Torque();
		glm::vec3 angularVelocity = physics.AngularVelocity();

		angularVelocity += angularAcceleration * Scene::dt;
		physics.SetAngularVelocity(angularVelocity);
	}

	void SystemPhysics::AfterAction() {}
}