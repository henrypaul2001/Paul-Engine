#include "ComponentPathfinder.h"
#include "Entity.h"
namespace Engine {
	ComponentPathfinder::ComponentPathfinder(NavigationMap* navigationMap, const float moveSpeed, const float nextPositionDistance)
	{
		this->navigationMap = navigationMap;
		this->moveSpeed = moveSpeed;
		this->nextPositionDistanceCheck = nextPositionDistance;
		nextPosition = glm::vec3(0.0f);
		targetReached = true;
		entityCanMove = true;
	}

	ComponentPathfinder::~ComponentPathfinder()
	{

	}

	void ComponentPathfinder::Close()
	{

	}

	void ComponentPathfinder::FindPath(const glm::vec3& start, const glm::vec3& end)
	{
		bool success = navigationMap->FindPath(start, end, activePath);
		if (!success) {
			std::cout << owner->Name() << "::COMPONENTPATHFINDER::Couldn't find path from " << start.x << " " << start.y << " " << start.z << " -> " << end.x << " " << end.y << " " << end.z << std::endl;
			targetReached = true;
		}
		else {
			targetReached = false;
		}
	}
}