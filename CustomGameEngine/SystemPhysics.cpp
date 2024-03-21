#include "SystemPhysics.h"
#include "Scene.h"
#include <algorithm>
#include <iostream>
namespace Engine 
{
	SystemPhysics::SystemPhysics()
	{
		gravity = 9.8f;
		gravityAxis = glm::vec3(0.0f, 1.0f, 0.0f);
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

	void SystemPhysics::Acceleration(ComponentPhysics* physics)
	{
		float inverseMass = physics->InverseMass();
		glm::vec3 velocity = physics->Velocity();
		glm::vec3 acceleration = physics->Force() * inverseMass;

		// Apply gravity if component enables it and is not an immovable object
		if (physics->Gravity() && inverseMass > 0) {
			acceleration -= glm::vec3(gravityAxis.x * gravity, gravityAxis.y * gravity, gravityAxis.z * gravity) * Scene::dt;
		}

		velocity += acceleration * Scene::dt;
		physics->SetVelocity(velocity);
	}

	void SystemPhysics::Physics(ComponentTransform* transform, ComponentPhysics* physics)
	{
		Acceleration(physics);

		glm::vec3 position = transform->Position();
		glm::vec3 velocity = physics->Velocity();
		transform->SetLastPosition(position);

		position += velocity * Scene::dt;

		transform->SetPosition(position + velocity);

		// Fake drag
		//float damping = 0.75f * Scene::dt;
		//physics->SetVelocity(physics->Velocity() * damping);

		physics->ClearForces();
		//transform->SetLastPosition(transform->Position());
		//transform->SetPosition(transform->Position() + physics->Velocity() * Scene::dt);
		//std::cout << " position: " << transform->Position().x << ", " << transform->Position().y << ", " << transform->Position().z << ". last position: " << transform->LastPosition().x << ", " << transform->LastPosition().y << ", " << transform->LastPosition().z << std::endl;
	}
}