#include "SystemPathfinding.h"
#include "Scene.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	void SystemPathfinding::OnAction(const unsigned int entityID, ComponentTransform& transform, ComponentPathfinder& pathfinder)
	{
		SCOPE_TIMER("SystemPathfinding::OnAction()");
		if (!pathfinder.HasReachedTarget()) {
			// Check if entity has reached next position
			const glm::vec3& currentPosition = transform.GetWorldPosition();
			glm::vec3 nextPathPosition = glm::vec3(pathfinder.GetNextPosition().x, currentPosition.y, pathfinder.GetNextPosition().z);
			const float targetReachedDistance = pathfinder.GetNextPositionDistanceCheck();

			const float distanceToNextPositionSquared = glm::distance2(currentPosition, nextPathPosition);

			if (distanceToNextPositionSquared <= (targetReachedDistance * targetReachedDistance)) {
				// Next waypoint position reached, set next waypoint
				NavigationPath& path = pathfinder.GetActivePath();
				path.PopWaypointStack();

				const std::stack<glm::vec3>& waypoints = path.GetWaypointStack();

				if (waypoints.size() == 0) {
					// End of path reached
					pathfinder.SetTargetReached(true);
				}
				else {
					pathfinder.SetNextPosition(path.GetNextWaypoint());
				}
			}

			// Check if entity can be moved (is grounded, isn't stunned, etc)
			if (pathfinder.GetEntityCanMove() && !pathfinder.HasReachedTarget()) {
				// Move toward next position
				const float moveSpeed = pathfinder.GetMoveSpeed();
				nextPathPosition = glm::vec3(pathfinder.GetNextPosition().x, currentPosition.y, pathfinder.GetNextPosition().z);
				const glm::vec3 moveDirection = glm::normalize(nextPathPosition - currentPosition);
				transform.SetPosition(currentPosition + ((moveDirection * moveSpeed) * Scene::dt));
			}
		}
	}

	void SystemPathfinding::AfterAction() {}
}