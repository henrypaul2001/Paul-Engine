#pragma once
#include "SystemCollision.h"
namespace Engine {
    class SystemCollisionBoxAABB : public SystemCollision
    {
	private:
		const ComponentTypes BOX_MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_BOX);
		const ComponentTypes AABB_MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_AABB);

		CollisionData Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) override;
	public:
		SystemCollisionBoxAABB(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollisionBoxAABB();

		SystemTypes Name() override { return SYSTEM_COLLISION_BOX_AABB; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;
    };
}