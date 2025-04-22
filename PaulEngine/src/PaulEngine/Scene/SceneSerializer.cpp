#include "pepch.h"
#include "SceneSerializer.h"
#include "Entity.h"
#include "Components.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Asset/TextureImporter.h"

#include <yaml-cpp/yaml.h>

#include "PaulEngine/Utils/YamlConversions.h"

namespace PaulEngine
{
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
			out << YAML::Key << "Position" << YAML::Value << transformComponent.Position();
			out << YAML::Key << "Rotation" << YAML::Value << transformComponent.Rotation();
			out << YAML::Key << "Scale" << YAML::Value << transformComponent.Scale();

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
			out << YAML::Value << "TextureScale" << YAML::Value << spriteComponent.TextureScale;

			out << YAML::Key << "AtlasHandle" << YAML::Value << spriteComponent.TextureAtlas;
			out << YAML::Key << "SelectedSubTextureName" << YAML::Value << spriteComponent.SelectedSubTextureName;
		
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
			out << YAML::Key << "Type" << YAML::Value << (int)rb2d.Type();
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentBoxCollider2D>()) {
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;

			ComponentBoxCollider2D& box2D = entity.GetComponent<ComponentBoxCollider2D>();
			out << YAML::Key << "Size" << YAML::Value << box2D.Size();
			out << YAML::Key << "Density" << YAML::Value << box2D.Density();
			out << YAML::Key << "Friction" << YAML::Value << box2D.Friction();
			out << YAML::Key << "Restitution" << YAML::Value << box2D.Restitution();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentCircleCollider2D>()) {
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap;

			ComponentCircleCollider2D& circle2D = entity.GetComponent<ComponentCircleCollider2D>();
			out << YAML::Key << "Radius" << YAML::Value << circle2D.Radius();
			out << YAML::Key << "Density" << YAML::Value << circle2D.Density();
			out << YAML::Key << "Friction" << YAML::Value << circle2D.Friction();
			out << YAML::Key << "Restitution" << YAML::Value << circle2D.Restitution();

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentTextRenderer>()) {
			out << YAML::Key << "TextRendererComponent";
			out << YAML::BeginMap;

			ComponentTextRenderer& textComponent = entity.GetComponent<ComponentTextRenderer>();
			out << YAML::Key << "TextString" << YAML::Value << textComponent.TextString;
			out << YAML::Key << "FontHandle" << YAML::Value << textComponent.Font;
			out << YAML::Key << "Colour" << YAML::Value << textComponent.Colour;
			out << YAML::Key << "Kerning" << YAML::Value << textComponent.Kerning;
			out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.LineSpacing;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentMeshRenderer>()) {
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;

			ComponentMeshRenderer& meshComponent = entity.GetComponent<ComponentMeshRenderer>();
			out << YAML::Key << "MaterialHandle" << YAML::Value << meshComponent.MaterialHandle;
			out << YAML::Key << "MeshHandle" << YAML::Value << meshComponent.MeshHandle;
			out << YAML::Key << "DepthFunc" << YAML::Value << (int)meshComponent.DepthState.Func;
			out << YAML::Key << "DepthTest" << YAML::Value << meshComponent.DepthState.Test;
			out << YAML::Key << "DepthWrite" << YAML::Value << meshComponent.DepthState.Write;
			out << YAML::Key << "FaceCulling" << YAML::Value << (int)meshComponent.CullState;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ComponentDirectionalLight>()) {
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;

			ComponentDirectionalLight& lightComponent = entity.GetComponent<ComponentDirectionalLight>();
			out << YAML::Key << "Diffuse" << YAML::Value << lightComponent.Diffuse;
			out << YAML::Key << "Specular" << YAML::Value << lightComponent.Specular;
			out << YAML::Key << "Ambient" << YAML::Value << lightComponent.Ambient;

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

		std::error_code error;
		std::filesystem::create_directories(filepath.parent_path(), error);
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
					transform.m_Position = transformNode["Position"].as<glm::vec3>();
					transform.m_Rotation = transformNode["Rotation"].as<glm::vec3>();
					transform.m_Scale = transformNode["Scale"].as<glm::vec3>();
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
					spriteComponent.TextureScale = spriteNode["TextureScale"].as<glm::vec2>();

					spriteComponent.TextureAtlas = spriteNode["AtlasHandle"].as<AssetHandle>();
					spriteComponent.SelectedSubTextureName = spriteNode["SelectedSubTextureName"].as<std::string>();
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
					rb2dComponent.m_Type = (ComponentRigidBody2D::BodyType)rb2dNode["Type"].as<int>();
					rb2dComponent.m_FixedRotation = rb2dNode["FixedRotation"].as<bool>();
				}

				YAML::Node box2DNode = entity["BoxCollider2DComponent"];
				if (box2DNode) {
					ComponentBoxCollider2D& box2DComponent = deserializedEntity.AddComponent<ComponentBoxCollider2D>();
					box2DComponent.m_Size = box2DNode["Size"].as<glm::vec2>();

					box2DComponent.m_Density = box2DNode["Density"].as<float>();
					box2DComponent.m_Friction = box2DNode["Friction"].as<float>();
					box2DComponent.m_Restitution = box2DNode["Restitution"].as<float>();
				}

				YAML::Node circle2DNode = entity["CircleCollider2DComponent"];
				if (circle2DNode) {
					ComponentCircleCollider2D& circle2DComponent = deserializedEntity.AddComponent<ComponentCircleCollider2D>();
					circle2DComponent.m_Radius = circle2DNode["Radius"].as<float>();

					circle2DComponent.m_Density = circle2DNode["Density"].as<float>();
					circle2DComponent.m_Friction = circle2DNode["Friction"].as<float>();
					circle2DComponent.m_Restitution = circle2DNode["Restitution"].as<float>();
				}

				YAML::Node textNode = entity["TextRendererComponent"];
				if (textNode) {
					ComponentTextRenderer& textComponent = deserializedEntity.AddComponent<ComponentTextRenderer>();
					textComponent.TextString = textNode["TextString"].as<std::string>();
					textComponent.Font = textNode["FontHandle"].as<uint64_t>();
					textComponent.Colour = textNode["Colour"].as<glm::vec4>();
					textComponent.Kerning = textNode["Kerning"].as<float>();
					textComponent.LineSpacing = textNode["LineSpacing"].as<float>();
				}

				YAML::Node meshNode = entity["MeshRendererComponent"];
				if (meshNode) {
					ComponentMeshRenderer& meshComponent = deserializedEntity.AddComponent<ComponentMeshRenderer>();
					meshComponent.MaterialHandle = meshNode["MaterialHandle"].as<AssetHandle>();
					meshComponent.MeshHandle = meshNode["MeshHandle"].as<AssetHandle>();
					meshComponent.DepthState.Func = (DepthFunc)meshNode["DepthFunc"].as<int>();
					meshComponent.DepthState.Test = meshNode["DepthTest"].as<bool>();
					meshComponent.DepthState.Write = meshNode["DepthWrite"].as<bool>();
					meshComponent.CullState = (FaceCulling)meshNode["FaceCulling"].as<int>();
				}

				YAML::Node lightNode = entity["DirectionalLightComponent"];
				if (lightNode) {
					ComponentDirectionalLight& lightComponent = deserializedEntity.AddComponent<ComponentDirectionalLight>();
					lightComponent.Diffuse = lightNode["Diffuse"].as<glm::vec3>();
					lightComponent.Specular = lightNode["Specular"].as<glm::vec3>();
					lightComponent.Ambient = lightNode["Ambient"].as<glm::vec3>();
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