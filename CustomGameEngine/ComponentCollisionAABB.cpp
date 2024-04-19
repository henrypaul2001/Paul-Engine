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

		isMovedByCollisions = true;

		ConstructCube();
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

	void ComponentCollisionAABB::ConstructCube()
	{
		// Vertices
		boundingBox.AddVertex(glm::vec3(localBounds.minX, localBounds.minY, localBounds.minZ)); // 0
		boundingBox.AddVertex(glm::vec3(localBounds.minX, localBounds.maxY, localBounds.minZ)); // 1
		boundingBox.AddVertex(glm::vec3(localBounds.maxX, localBounds.maxY, localBounds.minZ)); // 2
		boundingBox.AddVertex(glm::vec3(localBounds.maxX, localBounds.minY, localBounds.minZ)); // 3

		boundingBox.AddVertex(glm::vec3(localBounds.minX, localBounds.minY, localBounds.maxZ)); // 4
		boundingBox.AddVertex(glm::vec3(localBounds.minX, localBounds.maxY, localBounds.maxZ)); // 5
		boundingBox.AddVertex(glm::vec3(localBounds.maxX, localBounds.maxY, localBounds.maxZ)); // 6
		boundingBox.AddVertex(glm::vec3(localBounds.maxX, localBounds.minY, localBounds.maxZ)); // 7

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

	void ComponentCollisionAABB::Close()
	{
	}
}