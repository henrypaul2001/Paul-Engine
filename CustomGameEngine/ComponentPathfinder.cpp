#include "ComponentPathfinder.h"
namespace Engine {
	ComponentPathfinder::ComponentPathfinder(NavigationMap* navigationMap, const float moveSpeed, const float nextPositionDistance)
	{
		this->navigationMap = navigationMap;
		this->moveSpeed = moveSpeed;
		this->nextPositionDistance = nextPositionDistance;
		nextPosition = glm::vec3(0.0f);
	}

	ComponentPathfinder::~ComponentPathfinder()
	{

	}

	void ComponentPathfinder::Close()
	{

	}
}