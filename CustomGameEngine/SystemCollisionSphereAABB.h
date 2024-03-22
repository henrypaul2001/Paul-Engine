#pragma once
#include "SystemCollision.h"
#include "ComponentTransform.h"
#include "ComponentCollisionAABB.h"
#include "ComponentCollisionSphere.h"
namespace Engine {
	class SystemCollisionSphereAABB : public SystemCollision
	{
	private:
		const ComponentTypes SPHERE_MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_SPHERE);
		const ComponentTypes AABB_MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_AABB);

		bool Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) override;
	public:
		SystemCollisionSphereAABB(EntityManager* entityManager, CollisionManager* collisionManager);
		~SystemCollisionSphereAABB();

		SystemTypes Name() override { return SYSTEM_COLLISION_SPHERE_AABB; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	};
}