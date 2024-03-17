#pragma once
#include "ComponentCollision.h"
namespace Engine {
	struct AABBPoints {
		float minX;
		float minY;
		float minZ;
		float maxX;
		float maxY;
		float maxZ;
	};

	class ComponentCollisionAABB : public ComponentCollision
	{
	private:
		AABBPoints bounds;
	public:
		ComponentCollisionAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
		~ComponentCollisionAABB();

		AABBPoints GetBoundary() { return bounds; }

		void SetMinX(float minX) { bounds.minX = minX; }
		void SetMinY(float minY) { bounds.minY = minY; }
		void SetMinZ(float minZ) { bounds.minZ = minZ; }

		void SetMaxX(float maxX) { bounds.maxX = maxX; }
		void SetMaxY(float maxY) { bounds.maxY = maxY; }
		void SetMaxZ(float maxZ) { bounds.maxZ = maxZ; }

		ComponentTypes ComponentType() override { return COMPONENT_COLLISION_AABB; }
		void Close() override;
	};
}