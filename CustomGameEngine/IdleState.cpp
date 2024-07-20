#include "IdleState.h"
#include "Scene.h"
#include "StateMachine.h"
#include "ComponentStateController.h"
namespace Engine {
	IdleState::IdleState(const IdleState& old_state) : State(old_state.name)
	{
		this->isLookingAround = old_state.isLookingAround;
		this->isSteppingAround = old_state.isSteppingAround;

		this->startPosition = old_state.startPosition;
		this->startOrientation = old_state.startOrientation;

		this->targetOrientation = old_state.targetOrientation;
		this->targetPosition = old_state.targetPosition;

		this->moveSpeed = old_state.moveSpeed;
		this->rotateSpeed = old_state.rotateSpeed;

		this->secondsToWait = old_state.secondsToWait;
		this->secondsWaited = old_state.secondsWaited;
	}

	IdleState::IdleState() : State("Idle")
	{
		startPosition = glm::vec3();
		startOrientation = glm::quat();

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

	IdleState::~IdleState()
	{

	}

	void IdleState::Update()
	{
		Entity* owner = parentStateMachine->GetParentComponent()->GetOwner();
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
		State::Enter();
		Entity* owner = parentStateMachine->GetParentComponent()->GetOwner();
		if (owner && owner->GetTransformComponent()) {
			startPosition = owner->GetTransformComponent()->GetWorldPosition();
			startOrientation = owner->GetTransformComponent()->GetOrientation();

			targetOrientation = glm::quat();
			targetPosition = glm::vec3();

			isLookingAround = false;
			isSteppingAround = false;

			secondsWaited = 0.0f;

			owner->GetPathfinder()->Reset();
			DecideNextActivity();
		}
	}

	void IdleState::Exit()
	{
		State::Exit();
	}

	void IdleState::DecideNextActivity()
	{
		float randomNumber = Random(0.0f, 1.0f);

		if (randomNumber <= 0.33f) {
			std::cout << "Idle sub state: WAIT" << std::endl;
			isLookingAround = false;
			isSteppingAround = false;
			secondsWaited = 0.0f;
			name = "Idle - Wait";
		}
		else if (randomNumber > 0.33f && randomNumber <= 0.66f) {
			std::cout << "Idle sub state: LOOK" << std::endl;
			isLookingAround = true;
			isSteppingAround = false;

			// Create new target orientation
			float randomAngle = Random(-90.0f, 90.0f);
			targetOrientation = glm::angleAxis(glm::radians(randomAngle), glm::vec3(0.0f, 1.0f, 0.0f));
			name = "Idle - Look";
		}
		else {
			std::cout << "Idle sub state: WALK" << std::endl;
			isLookingAround = false;
			isSteppingAround = true;

			// Create new target position
			targetPosition = startPosition;

			float randomX = Random(-0.15f, 0.15f);
			float randomZ = Random(-0.15f, 0.15f);

			targetPosition.x += randomX;
			targetPosition.z += randomZ;
			name = "Idle - Walk";
		}
	}
}