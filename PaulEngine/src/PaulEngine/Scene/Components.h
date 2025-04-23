#pragma once
#include "PaulEngine/Core/UUID.h"
#include "PaulEngine/Renderer/Texture.h"
#include "PaulEngine/Renderer/Font.h"

#include <PaulEngine/Scene/SceneCamera.h>
#include "EntityScript.h"

#include "PaulEngine/Renderer/RenderPipeline.h"

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

		AssetHandle TextureAtlas = 0;
		std::string SelectedSubTextureName = "";

		Component2DSprite(const glm::vec4& colour = glm::vec4(1.0f)) : Colour(colour), TextureScale(glm::vec2(1.0f)) {}
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

		BodyType Type() const { return m_Type; }
		bool FixedRotation() const { return m_FixedRotation; }

		void SetType(BodyType type) { m_Type = type; m_PhysicsDirty = true; }
		void SetFixedRotation(bool fixed) { m_FixedRotation = fixed; m_PhysicsDirty = true; }

		struct B2RuntimeBody {
			int32_t index1;
			uint16_t world0;
			uint16_t generation;
		};
		const B2RuntimeBody& GetRuntimeBody() const { return m_RuntimeBody; }

	private:
		friend class Scene;
		friend class SceneSerializer;

		BodyType m_Type = BodyType::Static;
		bool m_FixedRotation = false;

		B2RuntimeBody m_RuntimeBody = { 0, 0, 0 };

		bool m_PhysicsDirty = false;
	};

	struct ComponentBoxCollider2D {
		glm::vec2 Size() const { return m_Size; }
		float Density() const { return m_Density; }
		float Friction() const { return m_Friction; }
		float Restitution() const { return m_Restitution; }

		struct B2RuntimeFixture {
			int32_t index1;
			uint16_t world0;
			uint16_t generation;
		};
		const B2RuntimeFixture& RuntimeFixture() const { return m_RuntimeFixture; }

		void SetSize(glm::vec2 size) { m_Size = size; m_PhysicsDirty = true; }
		void SetDensity(float density) { m_Density = density; m_PhysicsDirty = true; }
		void SetFriction(float friction) { m_Friction = friction; m_PhysicsDirty = true; }
		void SetRestitution(float restitution) { m_Restitution = restitution; m_PhysicsDirty = true; }

	private:
		friend class Scene;
		friend class SceneSerializer;

		glm::vec2 m_Size = glm::vec2(0.5f);

		float m_Density = 1.0f;
		float m_Friction = 0.5f;
		float m_Restitution = 0.0f;

		B2RuntimeFixture m_RuntimeFixture = { 0, 0, 0 };

		bool m_PhysicsDirty = false;
	};

	struct ComponentCircleCollider2D {
		float Radius() const { return m_Radius; }
		float Density() const { return m_Density; }
		float Friction() const { return m_Friction; }
		float Restitution() const { return m_Restitution; }

		struct B2RuntimeFixture {
			int32_t index1;
			uint16_t world0;
			uint16_t generation;
		};
		const B2RuntimeFixture& RuntimeFixture() const { return m_RuntimeFixture; }

		void SetRadius(float radius) { m_Radius = radius; m_PhysicsDirty = true; }
		void SetDensity(float density) { m_Density = density; m_PhysicsDirty = true; }
		void SetFriction(float friction) { m_Friction = friction; m_PhysicsDirty = true; }
		void SetRestitution(float restitution) { m_Restitution = restitution; m_PhysicsDirty = true; }

	private:
		friend class Scene;
		friend class SceneSerializer;

		float m_Radius = 0.5f;

		float m_Density = 1.0f;
		float m_Friction = 0.5f;
		float m_Restitution = 0.0f;

		B2RuntimeFixture m_RuntimeFixture = { 0, 0, 0 };
	
		bool m_PhysicsDirty = false;
	};

	struct ComponentTextRenderer {
		std::string TextString;
		AssetHandle Font = 0;
		glm::vec4 Colour = glm::vec4(1.0f);
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
	};

	struct ComponentMeshRenderer {
		AssetHandle MaterialHandle = 0;
		AssetHandle MeshHandle = 0;
		DepthState DepthState;
		FaceCulling CullState = FaceCulling::BACK;
	};

	struct ComponentDirectionalLight {
		glm::vec3 Diffuse = glm::vec3(0.5f);
		glm::vec3 Specular = glm::vec3(0.5f);
		glm::vec3 Ambient = glm::vec3(0.1f);
	};

	struct ComponentPointLight {
		float Radius = 25.0f;

		glm::vec3 Diffuse = glm::vec3(0.5f);
		glm::vec3 Specular = glm::vec3(0.5f);
		glm::vec3 Ambient = glm::vec3(0.1f);
	};
}