#include "SystemPhysics.h"
#include "Scene.h"
#include <algorithm>
#include <iostream>
namespace Engine 
{
	SystemPhysics::SystemPhysics() {}

	SystemPhysics::~SystemPhysics()
	{
		delete& MASK;
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

			ComponentVelocity* velocity = nullptr;
			for (Component* c : components) {
				velocity = dynamic_cast<ComponentVelocity*>(c);
				if (velocity != nullptr) {
					break;
				}
			}

			//std::cout << entity->Name();
			Motion(transform, velocity);
		}
	}

	void SystemPhysics::Motion(ComponentTransform* transform, ComponentVelocity* velocity)
	{
		transform->SetLastPosition(transform->Position());
		transform->SetPosition(transform->Position() + velocity->Velocity() * Scene::dt);
		//std::cout << " position: " << transform->Position().x << ", " << transform->Position().y << ", " << transform->Position().z << ". last position: " << transform->LastPosition().x << ", " << transform->LastPosition().y << ", " << transform->LastPosition().z << std::endl;
	}
}