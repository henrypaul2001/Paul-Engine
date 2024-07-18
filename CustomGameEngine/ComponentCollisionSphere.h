#pragma once
#include "Component.h"
#include "ComponentCollision.h"
#include <glm/ext/matrix_float4x4.hpp>
namespace Engine {
    class ComponentCollisionSphere : public ComponentCollision
    {
	private:
		float sphereRadius;
	public:
		ComponentCollisionSphere(const ComponentCollisionSphere& old_component);
		ComponentCollisionSphere(float sphereRadius);
		~ComponentCollisionSphere();

		Component* Copy() override { return new ComponentCollisionSphere(*this); }

		float CollisionRadius() { return sphereRadius; }
		void SetCollisionRadius(float radius) { sphereRadius = radius; }

		ComponentTypes ComponentType() override { return COMPONENT_COLLISION_SPHERE; }
		void Close() override;
    };
}
