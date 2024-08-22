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

		float GetBiggestExtent() const {
			float x = maxX - minX;
			float y = maxY - minY;
			float z = maxZ - minZ;
			float biggest = x;
			if (y > biggest) { biggest = y; }
			if (z > biggest) { biggest = z; }
			return biggest;
		}

		const std::vector<glm::vec3>& GetCorners() const {
			glm::vec3 minExtent = glm::vec3(minX, minY, minZ);
			glm::vec3 maxExtent = glm::vec3(maxX, maxY, maxZ);

			std::vector<glm::vec3> corners(8);
			corners[0] = glm::vec3(minExtent.x, minExtent.y, minExtent.z);
			corners[1] = glm::vec3(maxExtent.x, minExtent.y, minExtent.z);
			corners[2] = glm::vec3(minExtent.x, maxExtent.y, minExtent.z);
			corners[3] = glm::vec3(maxExtent.x, maxExtent.y, minExtent.z);
			corners[4] = glm::vec3(minExtent.x, minExtent.y, maxExtent.z);
			corners[5] = glm::vec3(maxExtent.x, minExtent.y, maxExtent.z);
			corners[6] = glm::vec3(minExtent.x, maxExtent.y, maxExtent.z);
			corners[7] = glm::vec3(maxExtent.x, maxExtent.y, maxExtent.z);

			return corners;
		}

		void TransformAABB(const glm::mat3& rotationMatrix, const glm::vec3& scale) {
			const std::vector<glm::vec3> corners = m->GetGeometryAABB().GetCorners();
			std::vector<glm::vec3> transformedCorners(8);

			for (int i = 0; i < 8; i++) {
				transformedCorners[i] = rotationMatrix * (corners[i] * scale);
			}

			glm::vec3 newMin = transformedCorners[0];
			glm::vec3 newMax = transformedCorners[0];

			// Find the new min/max extents
			for (int i = 1; i < 8; i++) {
				newMin = glm::min(newMin, transformedCorners[i]);
				newMax = glm::max(newMax, transformedCorners[i]);
			}

			minX = newMin.x;
			minY = newMin.y;
			minZ = newMin.z;
			maxX = newMax.x;
			maxY = newMax.y;
			maxZ = newMax.z;
		}

		AABBPoints(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : minX(minX), minY(minY), minZ(minZ), maxX(maxX), maxY(maxY), maxZ(maxZ) {}
		AABBPoints() : minX(-5.0f), minY(-5.0f), minZ(-5.0f), maxX(5.0f), maxY(5.0f), maxZ(5.0f) {}
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