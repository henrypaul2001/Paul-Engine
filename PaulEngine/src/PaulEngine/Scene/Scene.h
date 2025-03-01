#pragma once
#include <entt.hpp>
#include "PaulEngine/Core/Timestep.h"

#include <glm/glm.hpp>

namespace PaulEngine
{
	// Temporary test components
	struct ComponentTransform {
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		ComponentTransform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) : Position(position), Scale(scale) {}
	};
	struct Component2DSprite {
		glm::vec4 Colour = glm::vec4(1.0f);

		Component2DSprite(const glm::vec4& colour = glm::vec4(1.0f)) : Colour(colour) {}
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		// temp
		entt::registry& GetReg() { return m_Registry; }

		void OnUpdate(Timestep timestep);

	private:
		entt::registry m_Registry;
	};
}