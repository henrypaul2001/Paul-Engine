#pragma once
#include "PaulEngine/Core/UUID.h"
#include "PaulEngine/Renderer/Texture.h"
#include "PaulEngine/Renderer/Font.h"

#include <PaulEngine/Scene/SceneCamera.h>
#include "EntityScript.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace PaulEngine
{
	struct ComponentID
	{
		UUID ID;
	};

	struct ComponentTag {
		std::string Tag;

		ComponentTag(const std::string& tag = std::string()) : Tag(tag) {}
	};

	struct ComponentTransform {
	public:
		const glm::vec3& Position() const	{ return m_Position; }
		const glm::vec3& Rotation() const	{ return m_Rotation; }
		const glm::vec3&    Scale()	const	{ return m_Scale; }

		void SetPosition (const glm::vec3& position) { m_Position = position; m_PhysicsDirty = true; }
		void SetRotation (const glm::vec3& rotation) { m_Rotation = rotation; m_PhysicsDirty = true; }
		void SetScale	 (const glm::vec3& scale)	 { m_Scale = scale; m_PhysicsDirty = true; }

		ComponentTransform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) : m_Position(position), m_Rotation(rotation), m_Scale(scale), m_PhysicsDirty(false) {}

		glm::mat4 GetTransform() const {
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, m_Position);

			glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));

			transform *= rotation;

			transform = glm::scale(transform, m_Scale);
			return transform;
		}

	private:
		friend class Scene;
		friend class SceneSerializer;
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f); // Radians
		glm::vec3 m_Scale = glm::vec3(1.0f);
		bool m_PhysicsDirty;
	};

	struct ComponentCamera {
		SceneCamera Camera;
		// bool Primary = true;
		bool FixedAspectRatio = false;
	};

	struct Component2DSprite {
		glm::vec4 Colour = glm::vec4(1.0f);
		AssetHandle Texture = 0;
		glm::vec2 TextureScale = glm::vec2(1.0f);

		Component2DSprite(const glm::vec4& colour = glm::vec4(1.0f)) : Colour(colour), TextureScale(glm::vec2(1.0f)) {}
		Component2DSprite(const glm::vec4& colour, const Ref<Texture2D>& texture, const glm::vec2& textureScale = glm::vec2(1.0f)) : Colour(colour), TextureScale(textureScale) {}
	};

	struct Component2DCircle {
		glm::vec4 Colour = glm::vec4(1.0f);
		float Thickness = 1.0f;
		float Fade = 0.005f;

		Component2DCircle(const glm::vec4& colour = glm::vec4(1.0f)) : Colour(colour) {}
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

	struct ComponentRigidBody2D {
		enum class BodyType { Static = 0, Dynamic = 1, Kinematic = 2 };
		
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		struct B2RuntimeBody {
			int32_t index1;
			uint16_t world0;
			uint16_t generation;
		};
		B2RuntimeBody RuntimeBody = { 0, 0, 0 };
	};

	struct ComponentBoxCollider2D {
		glm::vec2 Offset = glm::vec2(0.0f);
		glm::vec2 Size = glm::vec2(0.5f);

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;

		struct B2RuntimeFixture {
			int32_t index1;
			uint16_t world0;
			uint16_t generation;
		};
		B2RuntimeFixture RuntimeFixture = { 0, 0, 0 };
	};

	struct ComponentCircleCollider2D {
		glm::vec2 Offset = glm::vec2(0.0f);
		float Radius = 0.5f;

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;

		struct B2RuntimeFixture {
			int32_t index1;
			uint16_t world0;
			uint16_t generation;
		};
		B2RuntimeFixture RuntimeFixture = { 0, 0, 0 };
	};

	struct ComponentTextRenderer {
		std::string TextString;
		AssetHandle Font = 0;
		glm::vec4 Colour = glm::vec4(1.0f);
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
	};
}