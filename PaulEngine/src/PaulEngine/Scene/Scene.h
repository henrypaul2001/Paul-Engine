#pragma once
#include <entt.hpp>
#include "PaulEngine/Core/Timestep.h"

#include <glm/glm.hpp>

namespace PaulEngine
{
	class Entity;
	// Temporary test components
	struct ComponentTag {
		std::string Tag;

		ComponentTag(const std::string& tag = std::string()) : Tag(tag) {}
	};
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

		Entity CreateEntity(const std::string& name = "Entity");

		void OnUpdate(Timestep timestep);

	private:
		friend class Entity;

		entt::registry m_Registry;
	};
}