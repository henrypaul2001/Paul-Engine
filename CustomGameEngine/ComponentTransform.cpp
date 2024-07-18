#include "ComponentTransform.h"
#include <glm/ext/matrix_transform.hpp>
#include "Entity.h"
#include <iostream>
#include "EntityManager.h"
namespace Engine
{
	ComponentTransform::ComponentTransform(const ComponentTransform& old_component)
	{
		this->owner = nullptr;

		this->position = old_component.position;
		this->lastPosition = old_component.lastPosition;
		this->rotationAxis = old_component.rotationAxis;
		this->rotationAngle = old_component.rotationAngle;
		this->scale = old_component.scale;
		this->forwardVector = old_component.forwardVector;

		this->orientation = old_component.orientation;

		this->worldModelMatrix = old_component.worldModelMatrix;

		this->parent = old_component.parent;

		// Copy children
		this->children.reserve(old_component.children.size());
		for (int i = 0; i < old_component.children.size(); i++) {
			children.push_back(new Entity(*old_component.children[i]));
			children[i]->GetEntityManager()->AddEntity(children[i]);
		}
	}

	ComponentTransform::ComponentTransform(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale) {
		this->position = position;
		this->lastPosition = position;
		this->rotationAxis = rotationAxis;
		this->rotationAngle = rotationAngle;
		this->scale = scale;
		
		orientation = glm::angleAxis(glm::radians(rotationAngle), rotationAxis);

		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));

		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(glm::vec3 position)
	{
		this->position = position;
		this->lastPosition = position;
		this->rotationAxis = glm::vec3(1.0, 1.0, 1.0);
		this->rotationAngle = 0.0f;
		this->scale = glm::vec3(1.0f, 1.0f, 1.0f);

		orientation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));

		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(float posX, float posY, float posZ) {
		this->position = glm::vec3(posX, posY, posZ);
		this->lastPosition = position;
		this->rotationAxis = glm::vec3(1.0, 1.0, 1.0);
		this->rotationAngle = 0.0f;
		this->scale = glm::vec3(1.0f, 1.0f, 1.0f);

		orientation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));

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
		SetOrientation(glm::angleAxis(glm::radians(rotationAngle), rotationAxis));
		//UpdateModelMatrix();
	}

	void ComponentTransform::SetScale(glm::vec3 scale) {
		this->scale = scale;
		UpdateModelMatrix();
	}

	void ComponentTransform::SetScale(float uniformScale)
	{
		this->scale = glm::vec3(uniformScale);
		UpdateModelMatrix();
	}

	void ComponentTransform::SetScale(float xScale, float yScale, float zScale)
	{
		this->scale = glm::vec3(xScale, yScale, zScale);
		UpdateModelMatrix();
	}

	void ComponentTransform::SetOrientation(glm::quat newOrientation)
	{
		orientation = newOrientation;
		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
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
		//glm::mat4 model(1.0f);
		//model = glm::translate(model, position);
		//model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
		//model = glm::rotate(model, );
		//model = glm::scale(model, scale);
		//worldModelMatrix = model;

		glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotate = glm::mat4_cast(orientation);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->scale);
		worldModelMatrix = translate * rotate * scale;

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

	glm::mat4 ComponentTransform::GetWorldModelMatrix()
	{
		return worldModelMatrix;
	}

	glm::vec3 ComponentTransform::GetWorldPosition()
	{
		glm::vec3 worldPos = Position();
		//worldPos = GetWorldModelMatrix() * glm::vec4(worldPos, 1.0);
		
		if (parent != nullptr) {
			worldPos += dynamic_cast<ComponentTransform*>(parent->GetComponent(COMPONENT_TRANSFORM))->GetWorldPosition();
		}

		return worldPos;
	}

	float ComponentTransform::GetBiggestScaleFactor()
	{
		float biggestScale = scale.x;
		if (scale.y > biggestScale) {
			biggestScale = scale.y;
		}
		else if (scale.z > biggestScale) {
			biggestScale = scale.z;
		}

		return biggestScale;
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