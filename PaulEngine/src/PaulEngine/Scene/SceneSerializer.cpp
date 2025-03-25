#include "pepch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/TextureImporter.h"

#include <yaml-cpp/yaml.h>

// Type conversions
namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs) {
			if (!node.IsSequence() || node.size() != 2) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<PaulEngine::UUID>
	{
		static Node encode(const PaulEngine::UUID& uuid) {
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, PaulEngine::UUID& uuid) {
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

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
		out << YAML::Key << "Entity" << YAML::Value << entity.UUID(); // Entity ID goes here (UUID)
		
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
			out << YAML::Key << "TextureHandle" << YAML::Value << spriteComponent.Texture;
			//out << YAML::Key << "TexturePath";
			//if (spriteComponent.Texture) {
			//	//const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(spriteComponent.Texture->Handle);
			//	//	std::string filepathString = spriteComponent.Texture->GetPath();
			//	//std::string filepathString = metadata.FilePath.string();
			//	//std::filesystem::path relativePath = std::filesystem::path(filepathString).lexically_relative(Project::GetAssetDirectory());
			//
			//	//out << YAML::Value << relativePath.string();
			//}
			//else {
			//	out << YAML::Value << "";
			//}
			out << YAML::Value << "TextureScale" << YAML::Value << spriteComponent.TextureScale;
		
			out << YAML::EndMap;
		}

		if (entity.HasComponent<Component2DCircle>()) {
			out << YAML::Key << "CircleComponent";
			out << YAML::BeginMap;

			Component2DCircle& circleComponent = entity.GetComponent<Component2DCircle>();
			out << YAML::Key << "Colour" << YAML::Value << circleComponent.Colour;
			out << YAML::Key << "Thickness" << YAML::Value << circleComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleComponent.Fade;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentRigidBody2D>()) {
			out << YAML::Key << "Rigid2DComponent";
			out << YAML::BeginMap;

			ComponentRigidBody2D& rb2d = entity.GetComponent<ComponentRigidBody2D>();
			out << YAML::Key << "Type" << YAML::Value << (int)rb2d.Type;
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentBoxCollider2D>()) {
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;

			ComponentBoxCollider2D& box2D = entity.GetComponent<ComponentBoxCollider2D>();
			out << YAML::Key << "Offset" << YAML::Value << box2D.Offset;
			out << YAML::Key << "Size" << YAML::Value << box2D.Size;
			out << YAML::Key << "Density" << YAML::Value << box2D.Density;
			out << YAML::Key << "Friction" << YAML::Value << box2D.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << box2D.Restitution;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentCircleCollider2D>()) {
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap;

			ComponentCircleCollider2D& circle2D = entity.GetComponent<ComponentCircleCollider2D>();
			out << YAML::Key << "Offset" << YAML::Value << circle2D.Offset;
			out << YAML::Key << "Radius" << YAML::Value << circle2D.Radius;
			out << YAML::Key << "Density" << YAML::Value << circle2D.Density;
			out << YAML::Key << "Friction" << YAML::Value << circle2D.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << circle2D.Restitution;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentTextRenderer>()) {
			out << YAML::Key << "TextRendererComponent";
			out << YAML::BeginMap;

			ComponentTextRenderer& textComponent = entity.GetComponent<ComponentTextRenderer>();
			out << YAML::Key << "TextString" << YAML::Value << textComponent.TextString;
			// out << FontAsset;
			out << YAML::Key << "Colour" << YAML::Value << textComponent.Colour;
			out << YAML::Key << "Kerning" << YAML::Value << textComponent.Kerning;
			out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.LineSpacing;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::SerializeYAML(const std::filesystem::path& filepath)
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

	void SceneSerializer::SerializeBinary(const std::filesystem::path& filepath)
	{
		PE_CORE_ASSERT(false, "Not yet implemented");
	}

	bool SceneSerializer::DeserializeYAML(const std::filesystem::path& filepath)
	{
		std::ifstream stream = std::ifstream(filepath);
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		if (!data["Scene"]) { return false; }

		std::string sceneName = data["Scene"].as<std::string>();
		PE_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		YAML::Node entities = data["Entities"];
		if (entities) {
			for (YAML::Node entity : entities) {
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				YAML::Node tagNode = entity["TagComponent"];
				if (tagNode) {
					name = tagNode["Tag"].as<std::string>();

					PE_CORE_TRACE("Deserializing entity with ID = {0}, name = {1}", uuid, name);
				}

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				YAML::Node transformNode = entity["TransformComponent"];
				if (transformNode) {
					ComponentTransform& transform = deserializedEntity.GetComponent<ComponentTransform>();
					transform.Position = transformNode["Position"].as<glm::vec3>();
					transform.Rotation = transformNode["Rotation"].as<glm::vec3>();
					transform.Scale = transformNode["Scale"].as<glm::vec3>();
				}

				YAML::Node cameraNode = entity["CameraComponent"];
				if (cameraNode) {
					ComponentCamera& cameraComponent = deserializedEntity.AddComponent<ComponentCamera>();
					SceneCamera& camera = cameraComponent.Camera;
					YAML::Node cameraProperties = cameraNode["Camera"];
					
					// Get properties
					SceneCameraType projectionType = (SceneCameraType)cameraProperties["ProjectionType"].as<int>();
					camera.m_PerspectiveVFOV = cameraProperties["PerspectiveFOV"].as<float>();
					camera.m_NearClip = cameraProperties["NearClip"].as<float>();
					camera.m_FarClip = cameraProperties["FarClip"].as<float>();
					camera.m_OrthographicSize = cameraProperties["OrthoSize"].as<float>();
					camera.m_AspectRatio = cameraProperties["AspectRatio"].as<float>();

					if (projectionType == SCENE_CAMERA_PERSPECTIVE) {
						camera.SetPerspective(camera.m_PerspectiveVFOV, camera.m_AspectRatio, camera.m_NearClip, camera.m_FarClip);
					}
					else {
						camera.SetOrthographic(camera.m_OrthographicSize, camera.m_AspectRatio, camera.m_NearClip, camera.m_FarClip);
					}

					cameraComponent.FixedAspectRatio = cameraNode["FixedAspect"].as<bool>();
				}

				YAML::Node spriteNode = entity["SpriteComponent"];
				if (spriteNode) {
					Component2DSprite& spriteComponent = deserializedEntity.AddComponent<Component2DSprite>();
					spriteComponent.Colour = spriteNode["Colour"].as<glm::vec4>();

					spriteComponent.Texture = spriteNode["TextureHandle"].as<AssetHandle>();

					//std::string filepath = spriteNode["TexturePath"].as<std::string>();
					//if (!filepath.empty()) {
					//	spriteComponent.Texture = TextureImporter::LoadTexture2D(Project::GetAssetFileSystemPath(filepath).string());
					//	//spriteComponent.Texture = AssetImporter::ImportAsset()
					//}
					spriteComponent.TextureScale = spriteNode["TextureScale"].as<glm::vec2>();
				}

				YAML::Node circleNode = entity["CircleComponent"];
				if (circleNode) {
					Component2DCircle& circleComponent = deserializedEntity.AddComponent<Component2DCircle>();
					circleComponent.Colour = circleNode["Colour"].as<glm::vec4>();
					circleComponent.Thickness = circleNode["Thickness"].as<float>();
					circleComponent.Fade = circleNode["Fade"].as<float>();
				}

				YAML::Node rb2dNode = entity["Rigid2DComponent"];
				if (rb2dNode) {
					ComponentRigidBody2D& rb2dComponent = deserializedEntity.AddComponent<ComponentRigidBody2D>();
					rb2dComponent.Type = (ComponentRigidBody2D::BodyType)rb2dNode["Type"].as<int>();
					rb2dComponent.FixedRotation = rb2dNode["FixedRotation"].as<bool>();
				}

				YAML::Node box2DNode = entity["BoxCollider2DComponent"];
				if (box2DNode) {
					ComponentBoxCollider2D& box2DComponent = deserializedEntity.AddComponent<ComponentBoxCollider2D>();
					box2DComponent.Offset = box2DNode["Offset"].as<glm::vec2>();
					box2DComponent.Size = box2DNode["Size"].as<glm::vec2>();

					box2DComponent.Density = box2DNode["Density"].as<float>();
					box2DComponent.Friction = box2DNode["Friction"].as<float>();
					box2DComponent.Restitution = box2DNode["Restitution"].as<float>();
				}

				YAML::Node circle2DNode = entity["CircleCollider2DComponent"];
				if (circle2DNode) {
					ComponentCircleCollider2D& circle2DComponent = deserializedEntity.AddComponent<ComponentCircleCollider2D>();
					circle2DComponent.Offset = circle2DNode["Offset"].as<glm::vec2>();
					circle2DComponent.Radius = circle2DNode["Radius"].as<float>();

					circle2DComponent.Density = circle2DNode["Density"].as<float>();
					circle2DComponent.Friction = circle2DNode["Friction"].as<float>();
					circle2DComponent.Restitution = circle2DNode["Restitution"].as<float>();
				}

				YAML::Node textNode = entity["TextRendererComponent"];
				if (textNode) {
					ComponentTextRenderer& textComponent = deserializedEntity.AddComponent<ComponentTextRenderer>();
					textComponent.TextString = textNode["TextString"].as<std::string>();
					textComponent.FontAsset = Font::GetDefault();
					textComponent.Colour = textNode["Colour"].as<glm::vec4>();
					textComponent.Kerning = textNode["Kerning"].as<float>();
					textComponent.LineSpacing = textNode["LineSpacing"].as<float>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeBinary(const std::filesystem::path& filepath)
	{
		PE_CORE_ASSERT(false, "Not yet implemented");
		return false;
	}
}