#include "pepch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"

#include <yaml-cpp/yaml.h>

namespace PaulEngine
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_Scene(scene) {}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << "128137235692"; // Entity ID goes here (UUID)
		
		// Components
		if (entity.HasComponent<ComponentTag>()) {
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			ComponentTag& tagComponent = entity.GetComponent<ComponentTag>();
			out << YAML::Key << "Tag" << YAML::Value << tagComponent.Tag;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentTransform>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			ComponentTransform& transformComponent = entity.GetComponent<ComponentTransform>();
			out << YAML::Key << "Position" << YAML::Value << transformComponent.Position;
			out << YAML::Key << "Rotation" << YAML::Value << transformComponent.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transformComponent.Scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentCamera>()) {
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			ComponentCamera& cameraComponent = entity.GetComponent<ComponentCamera>();
			SceneCamera& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.IsPerspective();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetVFOV();
			out << YAML::Key << "NearClip" << YAML::Value << camera.GetNearClip();
			out << YAML::Key << "FarClip" << YAML::Value << camera.GetFarClip();
			out << YAML::Key << "OrthoSize" << YAML::Value << camera.GetOrthoSize();
			out << YAML::Key << "AspectRatio" << YAML::Value << camera.GetAspectRatio();
			out << YAML::EndMap;

			out << YAML::Key << "FixedAspect" << YAML::Value << cameraComponent.FixedAspectRatio;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<Component2DSprite>()) {
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;

			Component2DSprite& spriteComponent = entity.GetComponent<Component2DSprite>();
			out << YAML::Key << "Colour" << YAML::Value << spriteComponent.Colour;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeYAML(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID) {
			Entity entity = Entity(entityID, m_Scene.get());
			if (!entity) { return; }

			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout = std::ofstream(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeBinary(const std::string& filepath)
	{
		PE_CORE_ASSERT(false, "Not yet implemented");
	}

	bool SceneSerializer::DeserializeYAML(const std::string& filepath)
	{
		return false;
	}

	bool SceneSerializer::DeserializeBinary(const std::string& filepath)
	{
		PE_CORE_ASSERT(false, "Not yet implemented");
		return false;
	}
}