#include "ComponentTransform.h"
namespace Engine
{
	ComponentTransform::ComponentTransform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
		this->position = position;
		this->lastPosition = position;
		this->rotation = rotation;
		this->scale = scale;
	}

	ComponentTransform::ComponentTransform(float posX, float posY, float posZ) {
		this->position = glm::vec3(posX, posY, posZ);
		this->lastPosition = position;
		this->rotation = glm::vec3(0, 0, 0);
		this->scale = glm::vec3(1, 1, 1);
	}

	ComponentTransform::~ComponentTransform() {
		delete& position;
		delete& lastPosition;
		delete& rotation;
		delete& scale;
	}

	void ComponentTransform::SetPosition(glm::vec3 position) {
		this->position = position;
	}

	void ComponentTransform::SetLastPosition(glm::vec3 position) {
		this->lastPosition = position;
	}

	void ComponentTransform::SetRotation(glm::vec3 rotation) {
		this->rotation = rotation;
	}

	void ComponentTransform::SetScale(glm::vec3 scale) {
		this->scale = scale;
	}

	void ComponentTransform::Close() {

	}
}