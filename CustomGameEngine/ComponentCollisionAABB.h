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
	};

	class ComponentCollisionAABB : public ComponentCollision
	{
	private:
		AABBPoints localBounds;
	public:
		ComponentCollisionAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, bool defaultCollisionResponse);
		~ComponentCollisionAABB();

		AABBPoints GetBoundary() { return localBounds; }

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