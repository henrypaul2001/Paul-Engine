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
		ComponentCollisionSphere(float sphereRadius, bool defaultCollisionResponse);
		~ComponentCollisionSphere();

		float CollisionRadius() { return sphereRadius; }
		void SetCollisionRadius(float radius) { sphereRadius = radius; }

		ComponentTypes ComponentType() override { return COMPONENT_COLLISION_SPHERE; }
		void Close() override;
    };
}
