#pragma once
#include "ComponentCollision.h"
#include <glm/ext/matrix_float4x4.hpp>
namespace Engine {
	struct AABBPoints {
		float minX;
		float minY;
		float minZ;
		float maxX;
		float maxY;
		float maxZ;

		float GetBiggestExtent() {
			float x = maxX - minX;
			float y = maxY - minY;
			float z = maxZ - minZ;
			float biggest = x;
			if (y > biggest) { biggest = y; }
			if (z > biggest) { biggest = z; }
			return biggest;
		}
	};

	class ComponentCollisionAABB : public ComponentCollision
	{
	private:
		AABBPoints localBounds;
		BoundingBox boundingBox;

		void ConstructCube();
	public:
		ComponentCollisionAABB(const ComponentCollisionAABB& old_component);
		ComponentCollisionAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
		~ComponentCollisionAABB();

		Component* Copy() override { return new ComponentCollisionAABB(*this); }

		AABBPoints GetBoundary() { return localBounds; }
		BoundingBox& GetBoundingBox() { return boundingBox; }

		void SetMinX(float minX) { localBounds.minX = minX; }
		void SetMinY(float minY) { localBounds.minY = minY; }
		void SetMinZ(float minZ) { localBounds.minZ = minZ; }

		void SetMaxX(float maxX) { localBounds.maxX = maxX; }
		void SetMaxY(float maxY) { localBounds.maxY = maxY; }
		void SetMaxZ(float maxZ) { localBounds.maxZ = maxZ; }

		AABBPoints GetWorldSpaceBounds(glm::mat4 modelMatrix);
		std::vector<glm::vec3> WorldSpacePoints(glm::mat4 modelMatrix);

		ComponentTypes ComponentType() override { return COMPONENT_COLLISION_AABB; }
		void Close() override;
	};
}