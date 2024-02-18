#pragma once
#include "Component.h"
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
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
		
		glm::mat4 worldModelMatrix;

		Entity* parent;
		std::vector<Entity*> children;
	public:
		ComponentTransform(glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale);
		ComponentTransform(float posX, float posY, float posZ);
		~ComponentTransform();

		void SetPosition(glm::vec3 position);
		void SetLastPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotationAxis, float rotationAngle);
		void SetScale(glm::vec3 scale);

		void UpdateModelMatrix();

		void SetParent(Entity* parent);

		glm::vec3 Position() { return position; }
		glm::vec3 LastPosition() { return lastPosition; }
		glm::vec3 RotationAxis() { return rotationAxis; }
		float RotationAngle() { return rotationAngle; }
		glm::vec3 Scale() { return scale; }
		glm::mat4 GetWorldModelMatrix() { return worldModelMatrix; }

		std::vector<Entity*> GetChildren() { return children; }
		Entity* FindChildWithName(std::string name);
		Entity* GetParent() { return parent; }
		void RemoveChild(Entity* entityPtr);
		void AddChild(Entity* entityPtr);

		ComponentTypes ComponentType() override { return COMPONENT_TRANSFORM; }
		void Close() override;
	};
}