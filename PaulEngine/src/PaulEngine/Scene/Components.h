#pragma once
#include "PaulEngine/Maths/Maths.h"
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
		glm::vec3 LocalPosition() const { return m_Position; }
		glm::vec3 LocalRotation() const { return m_Rotation; }
		glm::vec3    LocalScale() const { return m_Scale; }

		glm::vec3 WorldPosition()
		{
			glm::mat4 worldTransform = GetTransform();
			return worldTransform[3];
		}
		glm::vec3 WorldRotation()
		{
			glm::mat4 worldTransform = GetTransform();
			return Maths::GetRotation(worldTransform);
		}
		glm::vec3 WorldScale()
		{
			glm::mat4 worldTransform = GetTransform();
			return Maths::GetScale(worldTransform);
		}

		void SetLocalPosition(const glm::vec3& position) { m_Position = position; m_PhysicsDirty = true; m_TransformDirty = true; }
		void SetLocalRotation(const glm::vec3& rotation) { m_Rotation = rotation; m_PhysicsDirty = true; m_TransformDirty = true; }
		void SetLocalScale(const glm::vec3& scale)		 { m_Scale = scale;		  m_PhysicsDirty = true; m_TransformDirty = true; }

		void SetWorldTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		{
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, position);
			glm::mat4 matRotation = glm::toMat4(glm::quat(rotation));
			transform *= matRotation;
			transform = glm::scale(transform, scale);

			glm::mat4 parentTransform = GetParentTransform();
			glm::mat4 localTransform = glm::inverse(parentTransform) * transform;

			glm::vec3 newPosLocal;
			glm::vec3 newRotLocal;
			glm::vec3 newScaleLocal;
			Maths::DecomposeTransform(localTransform, newPosLocal, newRotLocal, newScaleLocal);

			SetLocalPosition(newPosLocal);
			SetLocalRotation(newRotLocal);
			SetLocalScale(newScaleLocal);
		}
		
		void SetWorldPosition(const glm::vec3& position)
		{
			SetWorldTransform(position, WorldRotation(), WorldScale());
		}
		void SetWorldRotation(const glm::vec3& rotation)
		{
			SetWorldTransform(WorldPosition(), rotation, WorldScale());
		}
		void SetWorldScale(const glm::vec3& scale)
		{
			SetWorldTransform(WorldPosition(), WorldRotation(), scale);
		}

		ComponentTransform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) : m_LocalTransform(glm::mat4(1.0f)), m_Position(position), m_Rotation(rotation), m_Scale(scale), m_PhysicsDirty(false), m_TransformDirty(true) {}

		glm::mat4 GetTransform()
		{
			glm::mat4 worldTransform = GetLocalTransform();
			if (m_Parent.IsValid())
			{
				worldTransform = m_Parent.GetComponent<ComponentTransform>().GetTransform() * worldTransform;
			}
			return worldTransform;
		}

		glm::mat4 GetLocalTransform()
		{
			if (m_TransformDirty) { UpdateLocalTransform(); }
			return m_LocalTransform;
		}
		glm::mat4 GetParentTransform()
		{
			if (m_Parent.IsValid())
			{
				return m_Parent.GetComponent<ComponentTransform>().GetTransform();
			}
			return glm::mat4(1.0f);
		}

		Entity GetParent() const { return m_Parent; }
		
		size_t NumChildren() const { return m_Children.size(); }
		const std::unordered_set<Entity>& GetChildren() const { return m_Children; }
		bool HasChild(Entity child) { return m_Children.contains(child); }

		static void SetParent(Entity child, Entity parent)
		{
			PE_CORE_ASSERT(child.IsValid(), "Invalid child entity");
			ComponentTransform& childTransform = child.GetComponent<ComponentTransform>();
			if (childTransform.GetParent() == parent) { return; }
			if (childTransform.HasChild(parent)) { return; }

			Entity previousParent = childTransform.GetParent();

			// Get current world vectors
			glm::vec3 worldPos;
			glm::vec3 worldRot;
			glm::vec3 worldScale;
			Maths::DecomposeTransform(childTransform.GetTransform(), worldPos, worldRot, worldScale);

			// Set new parent
			childTransform.m_Parent = parent;
			childTransform.m_PhysicsDirty = true; childTransform.m_TransformDirty = true;

			// Update children list in parent
			if (parent.IsValid())
			{
				parent.GetComponent<ComponentTransform>().m_Children.emplace(child);
			}

			// Remove child from previous parent
			if (previousParent.IsValid())
			{
				previousParent.GetComponent<ComponentTransform>().m_Children.erase(child);
			}

			// Update world position with previous world position
			childTransform.SetWorldTransform(worldPos, worldRot, worldScale);
		}

	private:
		void UpdateLocalTransform()
		{
			m_TransformDirty = false;
			m_LocalTransform = glm::mat4(1.0f);
			m_LocalTransform = glm::translate(m_LocalTransform, m_Position);

			glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));

			m_LocalTransform *= rotation;

			m_LocalTransform = glm::scale(m_LocalTransform, m_Scale);
		}

		void RemapEntityRelationships(Scene* scene)
		{
			if (scene) {
				if (m_Parent.IsValid())
				{
					m_Parent = scene->FindEntityWithUUID(m_Parent.UUID());
				}

				std::unordered_set<Entity> newChildren;
				for (auto it : m_Children)
				{
					UUID childID = it.UUID();
					newChildren.emplace(scene->FindEntityWithUUID(childID));
				}
				m_Children = newChildren;
			}
		}

		friend class Scene;
		friend class SceneSerializer;
		glm::mat4 m_LocalTransform = glm::mat4(1.0f);
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec3 m_Rotation = glm::vec3(0.0f); // Radians
		glm::vec3 m_Scale = glm::vec3(1.0f);
		bool m_PhysicsDirty = false;
		bool m_TransformDirty = true;

		Entity m_Parent = Entity();
		std::unordered_set<Entity> m_Children;
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

		bool CastShadows = true;
		float ShadowMinBias = 0.005f;
		float ShadowMaxBias = 0.05f;
		float ShadowMapCameraDistance = 20.0f;
		float ShadowMapProjectionSize = 20.0f;
		float ShadowMapNearClip = 0.01f;
		float ShadowMapFarClip = 150.0f;
	};

	struct ComponentPointLight {
		float Radius = 25.0f;

		glm::vec3 Diffuse = glm::vec3(0.5f);
		glm::vec3 Specular = glm::vec3(0.5f);
		glm::vec3 Ambient = glm::vec3(0.1f);

		bool CastShadows = true;
		float ShadowMinBias = 0.005f;
		float ShadowMaxBias = 0.05f;
		float ShadowMapNearClip = 0.01f;
		float ShadowMapFarClip = 50.0f;
	};

	struct ComponentSpotLight {
		float Range = 25.0f;
		float InnerCutoff = 20.0f;
		float OuterCutoff = 30.0f;

		glm::vec3 Diffuse = glm::vec3(0.5f);
		glm::vec3 Specular = glm::vec3(0.5f);
		glm::vec3 Ambient = glm::vec3(0.1f);

		bool CastShadows = true;
		float ShadowMinBias = 0.005f;
		float ShadowMaxBias = 0.05f;
		float ShadowMapNearClip = 0.01f;
		float ShadowMapFarClip = 50.0f;
	};
}