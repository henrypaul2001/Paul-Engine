#include "ComponentTransform.h"
namespace Engine
{
	ComponentTransform::ComponentTransform(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale) {
		this->position = position;
		this->lastPosition = position;
		this->rotationAxis = rotationAxis;
		this->rotationAngle = rotationAngle;
		this->scale = scale;
	}

	ComponentTransform::ComponentTransform(float posX, float posY, float posZ) {
		this->position = glm::vec3(posX, posY, posZ);
		this->lastPosition = position;
		this->rotationAxis = glm::vec3(1.0, 1.0, 1.0);
		this->rotationAngle = 0.0f;
		this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	ComponentTransform::~ComponentTransform() {

	}

	void ComponentTransform::SetPosition(glm::vec3 position) {
		this->position = position;
	}

	void ComponentTransform::SetLastPosition(glm::vec3 position) {
		this->lastPosition = position;
	}

	void ComponentTransform::SetRotationAxis(glm::vec3 rotationAxis) {
		this->rotationAxis = rotationAxis;
	}

	void ComponentTransform::SetRotationAngle(float rotationAngle) {
		this->rotationAngle = rotationAngle;
	}

	void ComponentTransform::SetScale(glm::vec3 scale) {
		this->scale = scale;
	}

	void ComponentTransform::Close() {

	}
}