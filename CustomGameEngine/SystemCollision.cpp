#include "SystemCollision.h"
#include "ComponentTransform.h"
namespace Engine {
	void SystemCollision::DefaultCollisionResponse(Entity* entity1, Entity* entity2)
	{
		ComponentTransform* transform1 = dynamic_cast<ComponentTransform*>(entity1->GetComponent(COMPONENT_TRANSFORM));
		transform1->SetPosition(transform1->LastPosition());

		ComponentTransform* transform2 = dynamic_cast<ComponentTransform*>(entity2->GetComponent(COMPONENT_TRANSFORM));
		transform2->SetPosition(transform2->LastPosition());
	}
}