#pragma once
#include "Component.h"
#include "NavigationPath.h"
#include "NavigationMap.h"
namespace Engine {
	class ComponentPathfinder : public Component
	{
	public:
		ComponentPathfinder(NavigationMap* navigationMap, const float moveSpeed = 0.5f, const float nextPositionDistance = 0.25f);
		~ComponentPathfinder();

		ComponentTypes ComponentType() override { return COMPONENT_PATHFINDER; }
		void Close() override;

		const NavigationMap& GetNavMap() const { return *navigationMap; }

		void SetMoveSpeed(const float newSpeed) { this->moveSpeed = newSpeed; }
		float GetMoveSpeed() const { return moveSpeed; }

		const glm::vec3& GetNextPosition() const { return nextPosition; }
		void SetNextPosition(const glm::vec3& nextPos) { this->nextPosition = nextPos; }
	private:
		float moveSpeed;
		float nextPositionDistance;

		NavigationMap* navigationMap;
		NavigationPath activePath;
		glm::vec3 nextPosition;
	};
}