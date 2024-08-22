#pragma once
#include "Component.h"
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "glm/gtx/quaternion.hpp"
namespace Engine
{
	class Entity; // forward declaration

	class ComponentTransform : public Component
	{
	private:
		glm::vec3 position;
		glm::vec3 lastPosition;
		glm::vec3 rotationAxis;
		float rotationAngle;
		glm::vec3 scale;
		glm::vec3 forwardVector;

		glm::quat orientation;

		glm::mat4 worldModelMatrix;

		Entity* parent;
		std::vector<Entity*> children;
	public:
		ComponentTransform(const ComponentTransform& old_component);
		ComponentTransform(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale);
		ComponentTransform(glm::vec3 position);
		ComponentTransform(float posX, float posY, float posZ);
		~ComponentTransform();

		Component* Copy() override { return new ComponentTransform(*this); }

		void SetPosition(glm::vec3 position);
		void SetLastPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotationAxis, float rotationAngle);
		void SetScale(glm::vec3 scale);
		void SetScale(float uniformScale);
		void SetScale(float xScale, float yScale, float zScale);

		void UpdateModelMatrix();

		void SetParent(Entity* parent);

		glm::vec3 Position() { return position; }
		glm::vec3 LastPosition() { return lastPosition; }
		glm::vec3 RotationAxis() { return rotationAxis; }
		float RotationAngle() { return rotationAngle; }
		glm::vec3 Scale() { return scale; }
		glm::mat4 GetWorldModelMatrix();
		const glm::vec3& GetForwardVector() const { return forwardVector; }

		glm::vec3 GetWorldPosition();

		float GetBiggestScaleFactor();

		void SetOrientation(glm::quat newOrientation);
		glm::quat GetOrientation() { return orientation; }

		std::vector<Entity*> GetChildren() { return children; }
		Entity* FindChildWithName(std::string name);
		Entity* GetParent() { return parent; }
		void RemoveChild(Entity* entityPtr);
		void AddChild(Entity* entityPtr);

		ComponentTypes ComponentType() override { return COMPONENT_TRANSFORM; }
		void Close() override;
		void OnAddedToEntity() override {
			UpdateModelMatrix();
		}
	};
}