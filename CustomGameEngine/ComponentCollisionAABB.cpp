#include "ComponentCollisionAABB.h"
namespace Engine {
	ComponentCollisionAABB::ComponentCollisionAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, bool defaultCollisionResponse)
	{
		localBounds = AABBPoints();
		localBounds.minX = minX;
		localBounds.minY = minY;
		localBounds.minZ = minZ;
		localBounds.maxX = maxX;
		localBounds.maxY = maxY;
		localBounds.maxZ = maxZ;

		useDefaultCollisionResponse = defaultCollisionResponse;
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

	std::vector<glm::vec3> ComponentCollisionAABB::WorldSpacePoints(glm::mat4 modelMatrix)
	{
		std::vector<glm::vec3> cubePoints = {
			glm::vec3(localBounds.minX, localBounds.minY, localBounds.maxZ), // front, bottom left point
			glm::vec3(localBounds.maxX, localBounds.minY, localBounds.maxZ), // front, bottom right point
			glm::vec3(localBounds.maxX, localBounds.maxY, localBounds.maxZ), // front, top right point
			glm::vec3(localBounds.minX, localBounds.maxY, localBounds.maxZ), // front, top left point

			glm::vec3(localBounds.minX, localBounds.minY, localBounds.minZ), // back, bottom left point
			glm::vec3(localBounds.maxX, localBounds.minY, localBounds.minZ), // back, bottom right point
			glm::vec3(localBounds.maxX, localBounds.maxY, localBounds.minZ), // back, top right point
			glm::vec3(localBounds.minX, localBounds.maxY, localBounds.minZ), // back, top left point
		};

		for (glm::vec3& point : cubePoints) {
			point = glm::vec3(modelMatrix * glm::vec4(point, 1.0f));
		}

		return cubePoints;
	}

	void ComponentCollisionAABB::Close()
	{
	}
}