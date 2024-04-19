#include "ComponentCollisionBox.h"
namespace Engine {
	ComponentCollisionBox::ComponentCollisionBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	{
		localExtents = BoxExtents();
		localExtents.minX = minX;
		localExtents.minY = minY;
		localExtents.minZ = minZ;
		localExtents.maxX = maxX;
		localExtents.maxY = maxY;
		localExtents.maxZ = maxZ;

		isMovedByCollisions = true;
		checkBroadPhaseFirst = true;
		ConstructCube();
	}

	ComponentCollisionBox::~ComponentCollisionBox()
	{

	}

	BoxExtents ComponentCollisionBox::GetWorldSpacePoints(glm::mat4 modelMatrix)
	{
		BoxExtents worldSpacePoints;

		worldSpacePoints.minX = localExtents.minX * modelMatrix[0][0] + localExtents.minY * modelMatrix[1][0] + localExtents.minZ * modelMatrix[2][0] + modelMatrix[3][0];
		worldSpacePoints.minY = localExtents.minX * modelMatrix[0][1] + localExtents.minY * modelMatrix[1][1] + localExtents.minZ * modelMatrix[2][1] + modelMatrix[3][1];
		worldSpacePoints.minZ = localExtents.minX * modelMatrix[0][2] + localExtents.minY * modelMatrix[1][2] + localExtents.minZ * modelMatrix[2][2] + modelMatrix[3][2];

		worldSpacePoints.maxX = localExtents.maxX * modelMatrix[0][0] + localExtents.maxY * modelMatrix[1][0] + localExtents.maxZ * modelMatrix[2][0] + modelMatrix[3][0];
		worldSpacePoints.maxY = localExtents.maxX * modelMatrix[0][1] + localExtents.maxY * modelMatrix[1][1] + localExtents.maxZ * modelMatrix[2][1] + modelMatrix[3][1];
		worldSpacePoints.maxZ = localExtents.maxX * modelMatrix[0][2] + localExtents.maxY * modelMatrix[1][2] + localExtents.maxZ * modelMatrix[2][2] + modelMatrix[3][2];

		return worldSpacePoints;
	}

	std::vector<glm::vec3> ComponentCollisionBox::WorldSpacePoints(glm::mat4 modelMatrix)
	{
		std::vector<glm::vec3> cubePoints = {
			glm::vec3(localExtents.minX, localExtents.minY, localExtents.maxZ), // front, bottom left point
			glm::vec3(localExtents.maxX, localExtents.minY, localExtents.maxZ), // front, bottom right point
			glm::vec3(localExtents.maxX, localExtents.maxY, localExtents.maxZ), // front, top right point
			glm::vec3(localExtents.minX, localExtents.maxY, localExtents.maxZ), // front, top left point

			glm::vec3(localExtents.minX, localExtents.minY, localExtents.minZ), // back, bottom left point
			glm::vec3(localExtents.maxX, localExtents.minY, localExtents.minZ), // back, bottom right point
			glm::vec3(localExtents.maxX, localExtents.maxY, localExtents.minZ), // back, top right point
			glm::vec3(localExtents.minX, localExtents.maxY, localExtents.minZ), // back, top left point
		};

		for (glm::vec3& point : cubePoints) {
			point = glm::vec3(modelMatrix * glm::vec4(point, 1.0f));
		}

		return cubePoints;
	}

	void ComponentCollisionBox::ConstructCube()
	{
		// Vertices
		boundingBox.AddVertex(glm::vec3(localExtents.minX, localExtents.minY, localExtents.minZ)); // 0
		boundingBox.AddVertex(glm::vec3(localExtents.minX, localExtents.maxY, localExtents.minZ)); // 1
		boundingBox.AddVertex(glm::vec3(localExtents.maxX, localExtents.maxY, localExtents.minZ)); // 2
		boundingBox.AddVertex(glm::vec3(localExtents.maxX, localExtents.minY, localExtents.minZ)); // 3

		boundingBox.AddVertex(glm::vec3(localExtents.minX, localExtents.minY, localExtents.maxZ)); // 4
		boundingBox.AddVertex(glm::vec3(localExtents.minX, localExtents.maxY, localExtents.maxZ)); // 5
		boundingBox.AddVertex(glm::vec3(localExtents.maxX, localExtents.maxY, localExtents.maxZ)); // 6
		boundingBox.AddVertex(glm::vec3(localExtents.maxX, localExtents.minY, localExtents.maxZ)); // 7

		// Indices, counter clockwise
		int face1[] = { 0, 1, 2, 3 };
		int face2[] = { 7, 6, 5, 4 };
		int face3[] = { 5, 6, 2, 1 };
		int face4[] = { 0, 3, 7, 4 };
		int face5[] = { 6, 7, 3, 2 };
		int face6[] = { 4, 5, 1, 0 };

		// Faces
		boundingBox.AddFace(glm::vec3(0.0f, 0.0f, -1.0f), 4, face1);
		boundingBox.AddFace(glm::vec3(0.0f, 0.0f, 1.0f), 4, face2);
		boundingBox.AddFace(glm::vec3(0.0f, 1.0f, 0.0f), 4, face3);
		boundingBox.AddFace(glm::vec3(0.0f, -1.0f, 0.0f), 4, face4);
		boundingBox.AddFace(glm::vec3(1.0f, 0.0f, 0.0f), 4, face5);
		boundingBox.AddFace(glm::vec3(-1.0f, 0.0f, 0.0f), 4, face6);
	}

	void ComponentCollisionBox::Close()
	{

	}
}