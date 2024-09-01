#pragma once
#include "SystemCollision.h"
namespace Engine {
    class SystemCollisionBox : public SystemCollision
    {
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_BOX);

		CollisionData Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) override;
	public:
		SystemCollisionBox(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollisionBox();

		SystemTypes Name() override { return SYSTEM_COLLISION_BOX; }
		void Run(const std::vector<Entity*>& entityList) override;
		void OnAction(Entity* entity) override;
		void AfterAction() override;
    };
}