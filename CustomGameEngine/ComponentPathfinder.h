#pragma once
#include "Component.h"
#include "NavigationPath.h"
#include "NavigationMap.h"
#include <iostream>
namespace Engine {
	class ComponentPathfinder : public Component
	{
	public:
		ComponentPathfinder(NavigationMap* navigationMap, const float moveSpeed = 0.05f, const float nextPositionDistance = 0.025f);
		~ComponentPathfinder();

		ComponentTypes ComponentType() override { return COMPONENT_PATHFINDER; }
		void Close() override;

		const NavigationMap& GetNavMap() const { return *navigationMap; }

		void SetMoveSpeed(const float newSpeed) { this->moveSpeed = newSpeed; }
		float GetMoveSpeed() const { return moveSpeed; }

		const float GetNextPositionDistanceCheck() const { return nextPositionDistanceCheck; }
		void SetNextPositionDistanceCheck(const float newDistance) { this->nextPositionDistanceCheck = newDistance; }

		const glm::vec3& GetNextPosition() const { return nextPosition; }
		void SetNextPosition(const glm::vec3& nextPos) { this->nextPosition = nextPos; }

		const bool FindPath(const glm::vec3& start, const glm::vec3& end);

		NavigationPath& GetActivePath() { return activePath; }

		void SetTargetReached(const bool targetReached) { this->targetReached = targetReached; }
		const bool HasReachedTarget() const { return targetReached; }

		void SetEntityCanMove(const bool canMove) { this->entityCanMove = canMove; }
		const bool GetEntityCanMove() const { return entityCanMove; }

	private:
		float moveSpeed;
		float nextPositionDistanceCheck;
		bool targetReached;
		bool entityCanMove;

		NavigationMap* navigationMap;
		NavigationPath activePath;
		glm::vec3 nextPosition;
	};
}