#include "ComponentCollisionBox.h"
namespace Engine {
	ComponentCollisionBox::ComponentCollisionBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ, bool defaultCollisionResponse)
	{
		localPoints = BoxPoints();
		localPoints.minX = minX;
		localPoints.minY = minY;
		localPoints.minZ = minZ;
		localPoints.maxX = maxX;
		localPoints.maxY = maxY;
		localPoints.maxZ = maxZ;

		isMovedByCollisions = true;

		useDefaultCollisionResponse = defaultCollisionResponse;
	}

	ComponentCollisionBox::~ComponentCollisionBox()
	{

	}

	BoxPoints ComponentCollisionBox::GetWorldSpacePoints(glm::mat4 modelMatrix)
	{
		BoxPoints worldSpacePoints;

		worldSpacePoints.minX = localPoints.minX * modelMatrix[0][0] + localPoints.minY * modelMatrix[1][0] + localPoints.minZ * modelMatrix[2][0] + modelMatrix[3][0];
		worldSpacePoints.minY = localPoints.minX * modelMatrix[0][1] + localPoints.minY * modelMatrix[1][1] + localPoints.minZ * modelMatrix[2][1] + modelMatrix[3][1];
		worldSpacePoints.minZ = localPoints.minX * modelMatrix[0][2] + localPoints.minY * modelMatrix[1][2] + localPoints.minZ * modelMatrix[2][2] + modelMatrix[3][2];

		worldSpacePoints.maxX = localPoints.maxX * modelMatrix[0][0] + localPoints.maxY * modelMatrix[1][0] + localPoints.maxZ * modelMatrix[2][0] + modelMatrix[3][0];
		worldSpacePoints.maxY = localPoints.maxX * modelMatrix[0][1] + localPoints.maxY * modelMatrix[1][1] + localPoints.maxZ * modelMatrix[2][1] + modelMatrix[3][1];
		worldSpacePoints.maxZ = localPoints.maxX * modelMatrix[0][2] + localPoints.maxY * modelMatrix[1][2] + localPoints.maxZ * modelMatrix[2][2] + modelMatrix[3][2];

		return worldSpacePoints;
	}

	std::vector<glm::vec3> ComponentCollisionBox::WorldSpacePoints(glm::mat4 modelMatrix)
	{
		std::vector<glm::vec3> cubePoints = {
			glm::vec3(localPoints.minX, localPoints.minY, localPoints.maxZ), // front, bottom left point
			glm::vec3(localPoints.maxX, localPoints.minY, localPoints.maxZ), // front, bottom right point
			glm::vec3(localPoints.maxX, localPoints.maxY, localPoints.maxZ), // front, top right point
			glm::vec3(localPoints.minX, localPoints.maxY, localPoints.maxZ), // front, top left point

			glm::vec3(localPoints.minX, localPoints.minY, localPoints.minZ), // back, bottom left point
			glm::vec3(localPoints.maxX, localPoints.minY, localPoints.minZ), // back, bottom right point
			glm::vec3(localPoints.maxX, localPoints.maxY, localPoints.minZ), // back, top right point
			glm::vec3(localPoints.minX, localPoints.maxY, localPoints.minZ), // back, top left point
		};

		for (glm::vec3& point : cubePoints) {
			point = glm::vec3(modelMatrix * glm::vec4(point, 1.0f));
		}

		return cubePoints;
	}

	void ComponentCollisionBox::Close()
	{

	}
}