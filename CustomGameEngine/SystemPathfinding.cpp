#include "SystemPathfinding.h"
namespace Engine {
	SystemPathfinding::SystemPathfinding()
	{

	}

	SystemPathfinding::~SystemPathfinding()
	{

	}

	void SystemPathfinding::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			std::vector<Component*> components = entity->Components();

			ComponentTransform* transform = nullptr;
			for (Component* c : components) {
				transform = dynamic_cast<ComponentTransform*>(c);
				if (transform != nullptr) {
					break;
				}
			}

			ComponentPathfinder* pathfinder = nullptr;
			for (Component* c : components) {
				pathfinder = dynamic_cast<ComponentPathfinder*>(c);
				if (pathfinder != nullptr) {
					break;
				}
			}

			Update(transform, pathfinder);
		}
	}

	void SystemPathfinding::AfterAction()
	{

	}

	void SystemPathfinding::Update(ComponentTransform* transform, ComponentPathfinder* pathfinder)
	{
		if (!pathfinder->HasReachedTarget()) {
			// Check if entity has reached next position
			glm::vec3 currentPosition = transform->GetWorldPosition();
			glm::vec3 nextPathPosition = pathfinder->GetNextPosition();
			float targetReachedDistance = pathfinder->GetNextPositionDistanceCheck();

			float distanceToNextPositionSquared = glm::distance2(currentPosition, nextPathPosition);

			if (distanceToNextPositionSquared <= (targetReachedDistance * targetReachedDistance)) {
				// Next waypoint position reached, set next waypoint
				NavigationPath& path = pathfinder->GetActivePath();
				path.PopWaypointStack();

				const std::stack<glm::vec3>& waypoints = path.GetWaypointStack();

				if (waypoints.size() == 0) {
					// End of path reached
					pathfinder->SetTargetReached(true);
				}
				else {
					pathfinder->SetNextPosition(path.GetNextWaypoint());
				}
			}

			// Check if entity can be moved (is grounded, isn't stunned, etc)
			if (pathfinder->GetEntityCanMove()) {
				// Move toward next position
				float moveSpeeed = pathfinder->GetMoveSpeed();
				glm::vec3 moveDirection = glm::normalize(nextPathPosition - currentPosition);
				transform->SetPosition(currentPosition + glm::vec3(moveDirection.x * moveSpeeed, 0.0f, moveDirection.z * moveSpeeed));
			}
		}
	}
}