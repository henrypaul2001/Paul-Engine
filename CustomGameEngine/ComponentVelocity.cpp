#include "ComponentVelocity.h"
namespace Engine
{
	ComponentVelocity::ComponentVelocity(float x, float y, float z)
	{
		velocity = glm::vec3(x, y, z);
	}

	ComponentVelocity::ComponentVelocity(glm::vec3 velocity)
	{
		this->velocity = velocity;
	}

	ComponentVelocity::~ComponentVelocity()
	{
		delete& velocity;
	}

	void ComponentVelocity::SetVelocity(glm::vec3 velocity)
	{
		this->velocity = velocity;
	}

	void ComponentVelocity::Close()
	{
	}
}