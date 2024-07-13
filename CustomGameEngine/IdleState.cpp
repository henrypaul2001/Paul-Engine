#include "IdleState.h"
#include "Scene.h"
namespace Engine {
	IdleState::IdleState(Entity* owner) : State("Idle")
	{
		if (owner && owner->GetTransformComponent()) {
			this->owner = owner;

			startPosition = owner->GetTransformComponent()->GetWorldPosition();
			startOrientation = owner->GetTransformComponent()->GetOrientation();

			targetOrientation = glm::quat();
			targetPosition = glm::vec3();
			isLookingAround = false;
			isSteppingAround = false;
			moveSpeed = 0.05f;
			secondsToWait = 2.5f;
			secondsWaited = 0.0f;
		}
		else {
			std::cout << "ERROR::IdleState::Cannot assign null entity to idle state" << std::endl;
			throw std::invalid_argument("ERROR::IdleState::Cannot assign null entity to idle state");
		}
	}

	IdleState::~IdleState()
	{

	}

	void IdleState::Update()
	{
		if (owner && owner->GetTransformComponent()) {
			ComponentTransform* transform = owner->GetTransformComponent();
			if (isLookingAround) {
				// Rotate towards target orientation

				// not yet implemented

				isLookingAround = false;
				DecideNextActivity();
			}
			else if (isSteppingAround) {

				// Check if reached target
				glm::vec3 currentPosition = transform->GetWorldPosition();
				float distanceToTargetSquared = glm::distance2(currentPosition, targetPosition);

				if (distanceToTargetSquared >= (moveSpeed * moveSpeed)) {
					// Move towards target
					glm::vec3 moveDirection = glm::normalize(targetPosition - currentPosition);
					transform->SetPosition(currentPosition + ((moveDirection * moveSpeed) * Scene::dt));
				}
				else {
					isSteppingAround = false;
					DecideNextActivity();
				}
			}
			else {
				// Stand still for 'x' seconds
				secondsWaited += Scene::dt;

				if (secondsWaited >= secondsToWait) {
					DecideNextActivity();
				}
			}
		}
	}

	void IdleState::Enter()
	{
		DecideNextActivity();
	}

	void IdleState::Exit()
	{

	}

	void IdleState::DecideNextActivity()
	{
		float randomNumber = Random(0.0f, 1.0f);

		if (randomNumber <= 0.33f) {
			isLookingAround = false;
			isSteppingAround = false;
			secondsWaited = 0.0f;
		}
		else if (randomNumber > 0.33f && randomNumber <= 0.66f) {
			isLookingAround = true;
			isSteppingAround = false;

			// Create new target orientation
		}
		else {
			isLookingAround = false;
			isSteppingAround = true;

			// Create new target position
			targetPosition = startPosition;

			float randomX = Random(-0.15f, 0.15f);
			float randomZ = Random(-0.15f, 0.15f);

			targetPosition.x += randomX;
			targetPosition.z += randomZ;
		}
	}
}