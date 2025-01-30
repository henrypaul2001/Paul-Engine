#include "EntityManager.h"
#include "ComponentTransform.h"
namespace Engine
{
	ComponentTransform::ComponentTransform(EntityManager* owning_ecs, const glm::vec3& position, const glm::vec3& rotationAxis, const float rotationAngle, const glm::vec3& scale) : owning_ecs(owning_ecs), position(position), rotationAxis(rotationAxis), rotationAngle(rotationAngle), scale(scale), parentID(INVALID_ID)
	{
		assert(owning_ecs);
		orientation = glm::angleAxis(glm::radians(rotationAngle), rotationAxis);
		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(EntityManager* owning_ecs, const glm::vec3& position) : owning_ecs(owning_ecs), position(position), rotationAxis(0.0f, 1.0f, 0.0f), rotationAngle(0.0f), scale(1.0f), parentID(INVALID_ID)
	{
		assert(owning_ecs);
		orientation = glm::angleAxis(glm::radians(rotationAngle), rotationAxis);
		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(EntityManager* owning_ecs, const float posX, const float posY, const float posZ) : owning_ecs(owning_ecs), position(posX, posY, posZ), rotationAxis(0.0f, 1.0f, 0.0f), rotationAngle(0.0f), scale(1.0f), parentID(INVALID_ID)
	{
		assert(owning_ecs);
		orientation = glm::angleAxis(glm::radians(rotationAngle), rotationAxis);
		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		UpdateModelMatrix();
	}

	ComponentTransform::~ComponentTransform()
	{

	}

	void ComponentTransform::UpdateModelMatrix()
	{
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotate = glm::mat4_cast(orientation);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->scale);
		worldModelMatrix = translate * rotate * scale;

		if (parentID != INVALID_ID) {
			ComponentTransform* parentTransform = owning_ecs->GetComponent<ComponentTransform>(parentID);
			if (parentTransform) {
				worldModelMatrix = parentTransform->GetWorldModelMatrix() * worldModelMatrix;
			}
		}

		for (unsigned int child : childrenIDs) {
			ComponentTransform* childTransform = owning_ecs->GetComponent<ComponentTransform>(child);
			if (childTransform) {
				childTransform->UpdateModelMatrix();
			}
		}
	}

	ComponentTransform ComponentTransform::Clone() const
	{
		// Copy children
		ComponentTransform clonedTransform = *this;
		for (int i = 0; i < childrenIDs.size(); i++) {
			Entity* child = owning_ecs->Clone(childrenIDs[i]);
			clonedTransform.childrenIDs[i] = child->ID();
		}
		return clonedTransform;
	}

	const glm::vec3 ComponentTransform::GetWorldPosition() const
	{
		glm::vec3 worldPos = position;
		if (parentID != INVALID_ID) {
			ComponentTransform* parentTransform = owning_ecs->GetComponent<ComponentTransform>(parentID);
			if (parentTransform) {
				worldPos += parentTransform->GetWorldPosition();
			}
		}
		return worldPos;
	}

	const Entity* ComponentTransform::FindChildWithName(const std::string& name) const
	{
		for (unsigned int id : childrenIDs) {
			Entity* e = owning_ecs->Find(id);
			if (e->Name() == name) {
				return e;
			}
		}
		return nullptr;
	}

	void ComponentTransform::RemoveChild(const unsigned int entityID)
	{
		for (int i = 0; i < childrenIDs.size(); i++) {
			if (childrenIDs[i] == entityID) {
				childrenIDs.erase(childrenIDs.begin() + i);
				return;
			}
		}
	}

	void ComponentTransform::AddChild(const unsigned int entityID)
	{
		for (unsigned int child : childrenIDs) {
			if (child == entityID) { return; }
		}
		childrenIDs.push_back(entityID);
		owning_ecs->GetComponent<ComponentTransform>(entityID)->parentID = ownerID;
	}
}