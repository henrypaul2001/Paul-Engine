#include "SystemPathfinding.h"
#include "Scene.h"
#include <glm/gtx/norm.hpp>
namespace Engine {
	SystemPathfinding::SystemPathfinding()
	{

	}

	SystemPathfinding::~SystemPathfinding()
	{

	}

	void SystemPathfinding::Run(const std::vector<Entity*>& entityList)
	{
		SCOPE_TIMER("SystemPathfinding::Run");
		System::Run(entityList);
	}

	void SystemPathfinding::OnAction(Entity* entity)
	{
		if ((entity->Mask() & MASK) == MASK) {
			ComponentTransform* transform = entity->GetTransformComponent();
			ComponentPathfinder* pathfinder = entity->GetPathfinder();

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
			glm::vec3 currentPosition = glm::vec3(transform->GetWorldPosition().x, transform->GetWorldPosition().y, transform->GetWorldPosition().z);
			glm::vec3 nextPathPosition = glm::vec3(pathfinder->GetNextPosition().x, transform->GetWorldPosition().y, pathfinder->GetNextPosition().z);
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
			if (pathfinder->GetEntityCanMove() && !pathfinder->HasReachedTarget()) {
				// Move toward next position
				float moveSpeed = pathfinder->GetMoveSpeed();
				nextPathPosition = glm::vec3(pathfinder->GetNextPosition().x, transform->GetWorldPosition().y, pathfinder->GetNextPosition().z);
				glm::vec3 moveDirection = glm::normalize(nextPathPosition - currentPosition);
				transform->SetPosition(currentPosition + ((moveDirection * moveSpeed) * Scene::dt));
			}
		}
	}
}