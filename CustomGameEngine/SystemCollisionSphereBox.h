#pragma once
#include "SystemCollision.h"
namespace Engine {
    class SystemCollisionSphereBox : public SystemCollision
    {
	private:
		const ComponentTypes BOX_MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_BOX);
		const ComponentTypes SPHERE_MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_SPHERE);

		CollisionData Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) override;
	public:
		SystemCollisionSphereBox(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollisionSphereBox();

		SystemTypes Name() override { return SYSTEM_COLLISION_BOX_SPHERE; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;
    };
}