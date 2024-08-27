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

		float startMinX;
		float startMinY;
		float startMinZ;
		float startMaxX;
		float startMaxY;
		float startMaxZ;

		float GetBiggestExtent() const {
			float x = maxX - minX;
			float y = maxY - minY;
			float z = maxZ - minZ;
			float biggest = x;
			if (y > biggest) { biggest = y; }
			if (z > biggest) { biggest = z; }
			return biggest;
		}

		std::vector<glm::vec3> GetStartCorners() const {
			glm::vec3 minExtent = glm::vec3(startMinX, startMinY, startMinZ);
			glm::vec3 maxExtent = glm::vec3(startMaxX, startMaxY, startMaxZ);

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

		std::vector<glm::vec3> GetCorners() const {
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
			std::vector<glm::vec3> corners = GetStartCorners();

			for (int i = 0; i < 8; i++) {
				corners[i] = rotationMatrix * (corners[i] * scale);
			}

			glm::vec3 newMin = corners[0];
			glm::vec3 newMax = corners[0];

			// Find the new min/max extents
			for (int i = 1; i < 8; i++) {
				newMin = glm::min(newMin, corners[i]);
				newMax = glm::max(newMax, corners[i]);
			}

			minX = newMin.x;
			minY = newMin.y;
			minZ = newMin.z;
			maxX = newMax.x;
			maxY = newMax.y;
			maxZ = newMax.z;
		}

		AABBPoints(float minX = -5.0f, float minY = -5.0f, float minZ = -5.0f, float maxX = 5.0f, float maxY = 5.0f, float maxZ = 5.0f) : minX(minX), minY(minY), minZ(minZ), maxX(maxX), maxY(maxY), maxZ(maxZ), startMinX(minX), startMinY(minY), startMinZ(minZ), startMaxX(maxX), startMaxY(maxY), startMaxZ(maxZ) {}
	
		bool operator==(AABBPoints const& aabb) {
			glm::vec3 thisMin = glm::vec3(minX, minY, minZ);
			glm::vec3 thisMax = glm::vec3(maxX, maxY, maxZ);

			glm::vec3 otherMin = glm::vec3(aabb.minX, aabb.minY, aabb.minZ);
			glm::vec3 otherMax = glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ);

			return (thisMin == otherMin && thisMax == otherMax);
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