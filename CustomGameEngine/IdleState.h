#pragma once
#include "State.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_float.hpp>
#include "Entity.h"
namespace Engine {
	class IdleState : public State
	{
	public:
		IdleState(Entity* owner);
		~IdleState();

		void Update() override;
		void Enter() override;
		void Exit() override;

	private:
		Entity* owner;

		bool isLookingAround;
		bool isSteppingAround;

		glm::vec3 startPosition;
		glm::quat startOrientation;

		glm::quat targetOrientation;
		glm::vec3 targetPosition;

		float moveSpeed;
		float rotateSpeed;

		float secondsToWait;
		float secondsWaited;

		float Random(float min, float max) const {
			return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
		}

		void DecideNextActivity();
	};
}