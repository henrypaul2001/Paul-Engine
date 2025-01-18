#pragma once
#include "ComponentCollision.h"
#include <glm/ext/matrix_float4x4.hpp>
namespace Engine {
    class ComponentCollisionSphere : public ComponentCollision
    {
	public:
		constexpr Engine::ColliderType ColliderType() const override { return COLLISION_SPHERE; }
		ComponentCollisionSphere(const float sphereRadius) : sphereRadius(sphereRadius) { isMovedByCollisions = true; }
		~ComponentCollisionSphere() {}

		float CollisionRadius() const { return sphereRadius; }
		void SetCollisionRadius(const float radius) { sphereRadius = radius; }

	private:
		float sphereRadius;
    };
}
