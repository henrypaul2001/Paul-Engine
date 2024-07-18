#pragma once
#include "Component.h"
#include <glm/vec3.hpp>
namespace Engine
{
	class ComponentVelocity : public Component
	{
	private:
		glm::vec3 velocity;

	public:
		ComponentVelocity(const ComponentVelocity& old_component);
		ComponentVelocity(float x, float y, float z);
		ComponentVelocity(glm::vec3 velocity);
		~ComponentVelocity();

		glm::vec3 Velocity() { return velocity; }
		void SetVelocity(glm::vec3 velocity);

		ComponentTypes ComponentType() override { return COMPONENT_VELOCITY; }
		void Close() override;

		Component* Copy() override { return new ComponentVelocity(*this); }
	};
}