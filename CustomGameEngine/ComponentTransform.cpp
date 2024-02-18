#include "ComponentTransform.h"
#include <glm/ext/matrix_transform.hpp>
#include "Entity.h"
#include <iostream>
namespace Engine
{
	ComponentTransform::ComponentTransform(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale) {
		this->position = position;
		this->lastPosition = position;
		this->rotationAxis = rotationAxis;
		this->rotationAngle = rotationAngle;
		this->scale = scale;
		
		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(float posX, float posY, float posZ) {
		this->position = glm::vec3(posX, posY, posZ);
		this->lastPosition = position;
		this->rotationAxis = glm::vec3(1.0, 1.0, 1.0);
		this->rotationAngle = 0.0f;
		this->scale = glm::vec3(1.0f, 1.0f, 1.0f);

		UpdateModelMatrix();
	}

	ComponentTransform::~ComponentTransform() {

	}

	void ComponentTransform::SetPosition(glm::vec3 position) {
		this->position = position;
		UpdateModelMatrix();
	}

	void ComponentTransform::SetLastPosition(glm::vec3 position) {
		this->lastPosition = position;
	}

	void ComponentTransform::SetRotation(glm::vec3 rotationAxis, float rotationAngle)
	{
		this->rotationAxis = rotationAxis;
		this->rotationAngle = rotationAngle;
		UpdateModelMatrix();
	}

	void ComponentTransform::SetScale(glm::vec3 scale) {
		this->scale = scale;
		UpdateModelMatrix();
	}

	void ComponentTransform::SetParent(Entity* parent)
	{
		if (this->parent != nullptr) {
			ComponentTransform* parentTransform = dynamic_cast<ComponentTransform*>(this->parent->GetComponent(COMPONENT_TRANSFORM));
			if (parentTransform != nullptr) {
				parentTransform->RemoveChild(GetOwner());
			}
		}

		this->parent = parent;

		ComponentTransform* parentTransform = dynamic_cast<ComponentTransform*>(this->parent->GetComponent(COMPONENT_TRANSFORM));
		if (parentTransform != nullptr) {
			parentTransform->AddChild(GetOwner());
		}

		UpdateModelMatrix();
	}

	void ComponentTransform::UpdateModelMatrix()
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
		model = glm::scale(model, scale);
		worldModelMatrix = model;

		if (parent != nullptr) {
			ComponentTransform* parentTransform = dynamic_cast<ComponentTransform*>(parent->GetComponent(COMPONENT_TRANSFORM));
			if (parentTransform != nullptr) {
				worldModelMatrix = parentTransform->GetWorldModelMatrix() * worldModelMatrix;
			}
		}

		for (Entity* e : children) {
			ComponentTransform* childTransform = dynamic_cast<ComponentTransform*>(e->GetComponent(COMPONENT_TRANSFORM));
			if (childTransform != nullptr) {
				childTransform->UpdateModelMatrix();
			}
		}
	}

	Entity* ComponentTransform::FindChildWithName(std::string name)
	{
		for (Entity* e : children) {
			if (e->Name() == name) {
				return e;
			}
		}
		return nullptr;
	}

	void ComponentTransform::RemoveChild(Entity* entityPtr)
	{
		for (int i = 0; i < children.size(); i++) {
			if (&children[i] == &entityPtr) {
				children.erase(children.begin() + i);
				return;
			}
		}
	}

	void ComponentTransform::AddChild(Entity* entityPtr)
	{
		children.push_back(entityPtr);
	}

	void ComponentTransform::Close() {

	}
}