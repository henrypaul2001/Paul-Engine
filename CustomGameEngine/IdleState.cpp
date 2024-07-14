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
			moveSpeed = 0.15f;
			rotateSpeed = 0.5f;
			secondsToWait = 2.5f;
			secondsWaited = 0.0f;

			srand(Scene::dt);
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
				glm::quat currentOrientation = transform->GetOrientation();

				float dotProduct = glm::dot(currentOrientation, targetOrientation);

				// if dot product is close to 1, then the orientations are closely the same
				if (dotProduct <= (1.0f - 0.1f)) {
					// Rotate current orientation towards target orientation at rotateSpeed
					glm::quat rotatedOrientation = glm::slerp(currentOrientation, targetOrientation, rotateSpeed * Scene::dt);
					transform->SetOrientation(rotatedOrientation);
				}
				else {
					isLookingAround = false;
					DecideNextActivity();
				}
			}
			else if (isSteppingAround) {
				// Check if reached target
				glm::vec3 currentPosition = transform->GetWorldPosition();
				float distanceToTargetSquared = glm::distance2(currentPosition, targetPosition);

				if (distanceToTargetSquared >= (moveSpeed * moveSpeed)) {
					// Rotate to face target position
					glm::quat currentOrientation = transform->GetOrientation();
					glm::quat targetRotation = glm::rotation(transform->GetForwardVector(), glm::normalize(targetPosition - currentPosition));

					glm::quat rotatedOrientation = glm::slerp(currentOrientation, targetOrientation, (rotateSpeed * 5.0f) * Scene::dt);
					transform->SetOrientation(rotatedOrientation);

					glm::vec3 forward = transform->GetForwardVector();

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
			std::cout << "Idle sub state: WAIT" << std::endl;
			isLookingAround = false;
			isSteppingAround = false;
			secondsWaited = 0.0f;
		}
		else if (randomNumber > 0.33f && randomNumber <= 0.66f) {
			std::cout << "Idle sub state: LOOK" << std::endl;
			isLookingAround = true;
			isSteppingAround = false;

			// Create new target orientation
			float randomAngle = Random(-90.0f, 90.0f);
			targetOrientation = glm::angleAxis(glm::radians(randomAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else {
			std::cout << "Idle sub state: WALK" << std::endl;
			isLookingAround = false;
			isSteppingAround = true;

			// Create new target position
			targetPosition = startPosition;

			float randomX = Random(-0.25f, 0.25f);
			float randomZ = Random(-0.25f, 0.25f);

			targetPosition.x += randomX;
			targetPosition.z += randomZ;
		}
	}
}