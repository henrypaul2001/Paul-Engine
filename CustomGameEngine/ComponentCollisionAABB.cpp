#include "ComponentCollisionAABB.h"
namespace Engine {
	ComponentCollisionAABB::ComponentCollisionAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	{
		localBounds = AABBPoints();
		localBounds.minX = minX;
		localBounds.minY = minY;
		localBounds.minZ = minZ;
		localBounds.maxX = maxX;
		localBounds.maxY = maxY;
		localBounds.maxZ = maxZ;
	}

	ComponentCollisionAABB::~ComponentCollisionAABB()
	{

	}

	AABBPoints ComponentCollisionAABB::GetWorldSpaceBounds(glm::mat4 modelMatrix)
	{
		AABBPoints worldBounds;

		// Get translation
		glm::vec3 translation = glm::vec3(modelMatrix[3]);

		// Get scale
		glm::vec3 scale;
		scale.x = glm::length(glm::vec3(modelMatrix[0]));
		scale.y = glm::length(glm::vec3(modelMatrix[1]));
		scale.z = glm::length(glm::vec3(modelMatrix[2]));

		// Transform local bounds into world space bounds
		worldBounds.minX = localBounds.minX * scale.x + translation.x;
		worldBounds.minY = localBounds.minY * scale.y + translation.y;
		worldBounds.minZ = localBounds.minZ * scale.z + translation.z;

		worldBounds.maxX = localBounds.maxX * scale.x + translation.x;
		worldBounds.maxY = localBounds.maxY * scale.y + translation.y;
		worldBounds.maxZ = localBounds.maxZ * scale.z + translation.z;

		return worldBounds;
	}

	void ComponentCollisionAABB::Close()
	{
	}
}