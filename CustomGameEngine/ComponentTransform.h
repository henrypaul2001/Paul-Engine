#pragma once
#include "Component.h"
#include <glm/vec3.hpp>
namespace Engine
{
	class ComponentTransform : Component
	{
	private:
		glm::vec3 position;
		glm::vec3 lastPosition;
		glm::vec3 rotation;
		glm::vec3 scale;

	public:
		ComponentTransform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
		ComponentTransform(float posX, float posY, float posZ);
		~ComponentTransform();

		void SetPosition(glm::vec3 position);
		void SetLastPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotation);
		void SetScale(glm::vec3 scale);

		glm::vec3 Position() { return position; }
		glm::vec3 LastPosition() { return lastPosition; }
		glm::vec3 Rotation() { return rotation; }
		glm::vec3 Scale() { return scale; }

		ComponentTypes ComponentType() override { return COMPONENT_TRANSFORM; }
		void Close() override;
	};
}