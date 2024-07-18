#include "ComponentPathfinder.h"
#include "Entity.h"
namespace Engine {
	ComponentPathfinder::ComponentPathfinder(const ComponentPathfinder& old_component)
	{
		this->owner = nullptr;

		this->moveSpeed = old_component.moveSpeed;
		this->nextPositionDistanceCheck = old_component.nextPositionDistanceCheck;
		this->targetReached = old_component.targetReached;
		this->entityCanMove = old_component.entityCanMove;

		this->navigationMap = old_component.navigationMap;
		this->activePath = old_component.activePath;
		this->nextPosition = old_component.nextPosition;
	}

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

	const bool ComponentPathfinder::FindPath(const glm::vec3& start, const glm::vec3& end)
	{
		bool success = navigationMap->FindPath(start, end, activePath);
		if (!success) {
			std::cout << owner->Name() << "::COMPONENTPATHFINDER::Couldn't find path from " << start.x << " " << start.y << " " << start.z << " -> " << end.x << " " << end.y << " " << end.z << std::endl;
			targetReached = true;
		}
		else {
			nextPosition = activePath.GetNextWaypoint();
			targetReached = false;
		}

		return success;
	}
}