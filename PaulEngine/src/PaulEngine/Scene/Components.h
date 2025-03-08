#pragma once
#include <PaulEngine/Scene/SceneCamera.h>
#include "EntityScript.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace PaulEngine
{
	struct ComponentTag {
		std::string Tag;

		ComponentTag(const std::string& tag = std::string()) : Tag(tag) {}
	};

	struct ComponentTransform {
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f); // Radians
		glm::vec3 Scale = glm::vec3(1.0f);

		ComponentTransform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) : Position(position), Rotation(rotation), Scale(scale) {}

		glm::mat4 GetTransform() const {
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, Position);

			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			transform *= rotation;

			transform = glm::scale(transform, Scale);
			return transform;
		}
	};

	struct ComponentCamera {
		SceneCamera Camera;
		// bool Primary = true;
		bool FixedAspectRatio = false;
	};

	struct Component2DSprite {
		glm::vec4 Colour = glm::vec4(1.0f);

		Component2DSprite(const glm::vec4& colour = glm::vec4(1.0f)) : Colour(colour) {}
	};

	struct ComponentNativeScript {
		EntityScript* Instance = nullptr;

		EntityScript* (*InstantiateScript)();
		void (*DestroyScript)(ComponentNativeScript*);

		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<EntityScript*>(new T()); };
			DestroyScript = [](ComponentNativeScript* scriptComponent) { delete scriptComponent->Instance; scriptComponent->Instance = nullptr; };
		}
	};
}