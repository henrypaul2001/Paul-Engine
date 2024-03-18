#pragma once
#include "SystemCollision.h"
#include "EntityManager.h"
#include "ComponentTransform.h"
#include "ComponentCollisionSphere.h"
namespace Engine {
	class SystemCollisionSphere : public SystemCollision
	{
	private:
		EntityManager* entityManager;
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_SPHERE);
	public:
		SystemCollisionSphere(EntityManager* entityManager);
		~SystemCollisionSphere();

		SystemTypes Name() override { return SYSTEM_COLLISION_SPHERE; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;

		void SphereCollision(ComponentTransform* transform, ComponentCollisionSphere* collider, ComponentTransform* transform2, ComponentCollisionSphere* collider2);
	};
}