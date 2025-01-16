#include "ComponentTransform.h"
#include "EntityManagerNew.h"
namespace Engine
{
	ComponentTransform::ComponentTransform(const ComponentTransform& old_component)
	{
		this->owning_ecs = old_component.owning_ecs;
		this->position = old_component.position;
		this->rotationAxis = old_component.rotationAxis;
		this->rotationAngle = old_component.rotationAngle;
		this->scale = old_component.scale;
		this->forwardVector = old_component.forwardVector;
		this->orientation = old_component.orientation;
		this->worldModelMatrix = old_component.worldModelMatrix;
		this->parent = old_component.parent;
		this->owner = nullptr;

		assert(owning_ecs);

		// Copy children
		this->children.reserve(old_component.children.size());
		for (int i = 0; i < old_component.children.size(); i++) {
			EntityNew* child = owning_ecs->Clone(old_component.children[i]->ID());
			AddChild(child);
		}
	}

	ComponentTransform::ComponentTransform(EntityManagerNew* owning_ecs, const glm::vec3& position, const glm::vec3& rotationAxis, const float rotationAngle, const glm::vec3& scale) : owning_ecs(owning_ecs), position(position), rotationAxis(rotationAxis), rotationAngle(rotationAngle), scale(scale), parent(nullptr)
	{
		assert(owning_ecs);
		orientation = glm::angleAxis(glm::radians(rotationAngle), rotationAxis);
		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(EntityManagerNew* owning_ecs, const glm::vec3& position) : owning_ecs(owning_ecs), position(position), rotationAxis(0.0f, 1.0f, 0.0f), rotationAngle(0.0f), scale(1.0f), parent(nullptr)
	{
		assert(owning_ecs);
		orientation = glm::angleAxis(glm::radians(rotationAngle), rotationAxis);
		forwardVector = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, 1.0f));
		UpdateModelMatrix();
	}

	ComponentTransform::ComponentTransform(EntityManagerNew* owning_ecs, const float posX, const float posY, const float posZ) : owning_ecs(owning_ecs), position(posX, posY, posZ), rotationAxis(0.0f, 1.0f, 0.0f), rotationAngle(0.0f), scale(1.0f), parent(nullptr)
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

		if (parent != nullptr) {
			ComponentTransform* parentTransform = owning_ecs->GetComponent<ComponentTransform>(parent->ID());
			if (parentTransform) {
				worldModelMatrix = parentTransform->GetWorldModelMatrix() * worldModelMatrix;
			}
		}

		for (EntityNew* child : children) {
			ComponentTransform* childTransform = owning_ecs->GetComponent<ComponentTransform>(child->ID());
			if (childTransform) {
				childTransform->UpdateModelMatrix();
			}
		}

		// TODO
		// Not possible with current design, and for good reason in all honesty. Components shouldn't need to know about other component types
		// For this reason, consider putting UpdateModelMatrix() into a small system, and maybe use a dirty flag if performance is a concern
		// Update owning entities geometry component if available
		//if (owner && owner->ContainsComponents(COMPONENT_GEOMETRY)) {
		//	owner->GetGeometryComponent()->GetModel()->UpdateGeometryBoundingBoxes(worldModelMatrix);
		//}
	}

	const glm::vec3 ComponentTransform::GetWorldPosition() const
	{
		glm::vec3 worldPos = position;
		if (parent != nullptr) {
			ComponentTransform* parentTransform = owning_ecs->GetComponent<ComponentTransform>(parent->ID());
			if (parentTransform) {
				worldPos += parentTransform->GetWorldPosition();
			}
		}
		return worldPos;
	}

	void ComponentTransform::RemoveChild(EntityNew* entityPtr)
	{
		for (int i = 0; i < children.size(); i++) {
			if (children[i] == entityPtr) {
				children.erase(children.begin() + i);
				return;
			}
		}
	}

	void ComponentTransform::AddChild(EntityNew* entityPtr)
	{
		for (EntityNew* child : children) {
			if (child == entityPtr) { return; }
		}
		children.push_back(entityPtr);
		owning_ecs->GetComponent<ComponentTransform>(entityPtr->ID())->parent = owner;
	}
}