#include "ComponentCollisionAABB.h"
namespace Engine {
	ComponentCollisionAABB::ComponentCollisionAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	{
		bounds = AABBPoints();
		bounds.minX = minX;
		bounds.minY = minY;
		bounds.minZ = minZ;
		bounds.maxX = maxX;
		bounds.maxY = maxY;
		bounds.maxZ = maxZ;
	}

	ComponentCollisionAABB::~ComponentCollisionAABB()
	{

	}

	void ComponentCollisionAABB::Close()
	{
	}
}