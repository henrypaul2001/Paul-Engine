#pragma once
#include "SystemCollision.h"
#include "ComponentTransform.h"
#include "ComponentCollisionSphere.h"
namespace Engine {
	class SystemCollisionSphere : public SystemCollision
	{
	private:
		const ComponentTypes MASK = (COMPONENT_TRANSFORM | COMPONENT_COLLISION_SPHERE);

		bool Intersect(ComponentTransform* transform, ComponentCollision* collider, ComponentTransform* transform2, ComponentCollision* collider2) override;
	public:
		SystemCollisionSphere(EntityManager* entityManager);
		~SystemCollisionSphere();

		SystemTypes Name() override { return SYSTEM_COLLISION_SPHERE; }
		void OnAction(Entity* entity) override;
		void AfterAction() override;
	};
}