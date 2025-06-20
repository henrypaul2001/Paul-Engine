#include "pepch.h"
#include "FrameRendererSerializer.h"

namespace PaulEngine
{
	// Primitive type dispatch table
	const std::unordered_map<std::type_index, DispatchSerializeFunc> FrameRendererSerializer::s_PrimitiveTypeSerializeDispatcher = {
		{ typeid(float),		[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<float>(c, o); }},
		{ typeid(glm::vec2),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::vec2>(c, o); }},
		{ typeid(glm::vec3),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::vec3>(c, o); }},
		{ typeid(glm::vec4),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::vec4>(c, o); }},

		{ typeid(int),			[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<int>(c, o); }},
		{ typeid(glm::ivec2),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::ivec2>(c, o); }},
		{ typeid(glm::ivec3),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::ivec3>(c, o); }},
		{ typeid(glm::ivec4),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::ivec4>(c, o); }},

		{ typeid(unsigned int), [](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<unsigned int>(c, o); }},
		{ typeid(glm::uvec2),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::uvec2>(c, o); }},
		{ typeid(glm::uvec3),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::uvec3>(c, o); }},
		{ typeid(glm::uvec4),	[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<glm::uvec4>(c, o); }},

		{ typeid(bool),			[](const IRenderComponentPrimitiveType* c, YAML::Emitter& o) { SerializePrimitiveComponent<bool>(c, o); }}
	};
	const std::unordered_map<std::type_index, DispatchDeserializeFunc> FrameRendererSerializer::s_PrimitiveTypeDeserializeDispatcher = {
		{ typeid(float),		[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<float>(c, n); }},
		{ typeid(glm::vec2),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::vec2>(c, n); }},
		{ typeid(glm::vec3),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::vec3>(c, n); }},
		{ typeid(glm::vec4),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::vec4>(c, n); }},

		{ typeid(int),			[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<int>(c, n); }},
		{ typeid(glm::ivec2),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::ivec2>(c, n); }},
		{ typeid(glm::ivec3),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::ivec3>(c, n); }},
		{ typeid(glm::ivec4),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::ivec4>(c, n); }},

		{ typeid(unsigned int), [](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<unsigned int>(c, n); }},
		{ typeid(glm::uvec2),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::uvec2>(c, n); }},
		{ typeid(glm::uvec3),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::uvec3>(c, n); }},
		{ typeid(glm::uvec4),	[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<glm::uvec4>(c, n); }},

		{ typeid(bool),			[](IRenderComponentPrimitiveType* c, const YAML::Node& n) { DeserializePrimitiveComponent<bool>(c, n); }}
	};

	void FrameRendererSerializer::SerializeComponent(const RenderComponentFramebuffer* framebufferComponent, YAML::Emitter& out)
	{
		PE_CORE_WARN("Framebuffer render component serialization not yet implemented");
	}
	void FrameRendererSerializer::SerializeComponent(const RenderComponentTexture* textureComponent, YAML::Emitter& out)
	{
		out << YAML::Key << "TextureAssetHandle" << YAML::Value << (UUID)textureComponent->TextureHandle;
	}
	void FrameRendererSerializer::SerializeComponent(const RenderComponentCamera* cameraComponent, YAML::Emitter& out)
	{
		PE_CORE_WARN("Camera render component serialization not yet implemented");
	}
	void FrameRendererSerializer::SerializeComponent(const RenderComponentMaterial* materialComponent, YAML::Emitter& out)
	{
		out << YAML::Key << "MaterialAssetHandle" << YAML::Value << (UUID)materialComponent->MaterialHandle;
	}
	void FrameRendererSerializer::SerializeComponent(const RenderComponentUBO* uboComponent, YAML::Emitter& out)
	{
		PE_CORE_WARN("UBO render component serialization not yet implemented");
	}
	void FrameRendererSerializer::SerializeComponent(const RenderComponentFBOAttachment* fboAttachmentComponent, YAML::Emitter& out)
	{
		PE_CORE_WARN("FBOAttachment render component serialization not yet implemented");
	}
	void FrameRendererSerializer::SerializeComponent(const RenderComponentEnvironmentMap* envMapComponent, YAML::Emitter& out)
	{
		out << YAML::Key << "EnvMapAssetHandle" << YAML::Value << (UUID)envMapComponent->EnvironmentHandle;
	}

	void FrameRendererSerializer::SerializeComponent(const IRenderComponentPrimitiveType* basePrimitive, YAML::Emitter& out)
	{
		std::type_index typeIndex = basePrimitive->GetPrimitiveTypeInfo();

		auto it = s_PrimitiveTypeSerializeDispatcher.find(typeIndex);
		if (it != s_PrimitiveTypeSerializeDispatcher.end())
		{
			it->second(basePrimitive, out);
		}
		else
		{
			PE_CORE_WARN("Unsupported RenderComponentPrimitiveType serialization");
		}
	}

	void FrameRendererSerializer::SerializeRenderComponent(const IRenderComponent* component, const char* name, YAML::Emitter& out)
	{
		const RenderComponentType type = component->GetType();
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << name;
		out << YAML::Key << "Type" << YAML::Value << RenderComponentTypeString(type);

		switch (type)
		{
		case RenderComponentType::None:
			out << YAML::Value << "Error type";
			break;
		case RenderComponentType::Framebuffer:
			SerializeComponent(static_cast<const RenderComponentFramebuffer*>(component), out);
			break;
		case RenderComponentType::Texture:
			SerializeComponent(static_cast<const RenderComponentTexture*>(component), out);
			break;
		case RenderComponentType::Camera:
			SerializeComponent(static_cast<const RenderComponentCamera*>(component), out);
			break;
		case RenderComponentType::Material:
			SerializeComponent(static_cast<const RenderComponentMaterial*>(component), out);
			break;
		case RenderComponentType::UBO:
			SerializeComponent(static_cast<const RenderComponentUBO*>(component), out);
			break;
		case RenderComponentType::FramebufferAttachment:
			SerializeComponent(static_cast<const RenderComponentFBOAttachment*>(component), out);
			break;
		case RenderComponentType::PrimitiveType:
			SerializeComponent(static_cast<const IRenderComponentPrimitiveType*>(component), out);
			break;
		case RenderComponentType::EnvironmentMap:
			SerializeComponent(static_cast<const RenderComponentEnvironmentMap*>(component), out);
			break;
		}

		out << YAML::EndMap;
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentFramebuffer* framebufferComponent, const YAML::Node& componentNode)
	{
		PE_CORE_WARN("Framebuffer render component serialization not yet implemented");
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentTexture* textureComponent, const YAML::Node& componentNode)
	{
		const YAML::Node& dataNode = componentNode["TextureAssetHandle"];
		if (dataNode)
		{
			textureComponent->TextureHandle = dataNode.as<UUID>();
		}
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentCamera* cameraComponent, const YAML::Node& componentNode)
	{
		PE_CORE_WARN("Camera render component serialization not yet implemented");
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentMaterial* materialComponent, const YAML::Node& componentNode)
	{
		const YAML::Node& dataNode = componentNode["MaterialAssetHandle"];
		if (dataNode)
		{
			materialComponent->MaterialHandle = dataNode.as<UUID>();
		}
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentUBO* uboComponent, const YAML::Node& componentNode)
	{
		PE_CORE_WARN("UBO render component serialization not yet implemented");
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentFBOAttachment* fboAttachmentComponent, const YAML::Node& componentNode)
	{
		PE_CORE_WARN("FBOAttachment render component serialization not yet implemented");
	}

	void FrameRendererSerializer::DeserializeComponent(RenderComponentEnvironmentMap* envMapComponent, const YAML::Node& componentNode)
	{
		const YAML::Node& dataNode = componentNode["EnvMapAssetHandle"];
		if (dataNode)
		{
			envMapComponent->EnvironmentHandle = dataNode.as<UUID>();
		}
	}

	void FrameRendererSerializer::DeserializeComponent(IRenderComponentPrimitiveType* basePrimitive, const YAML::Node& componentNode)
	{
		std::type_index typeIndex = basePrimitive->GetPrimitiveTypeInfo();

		auto it = s_PrimitiveTypeDeserializeDispatcher.find(typeIndex);
		if (it != s_PrimitiveTypeDeserializeDispatcher.end())
		{
			it->second(basePrimitive, componentNode);
		}
		else
		{
			PE_CORE_WARN("Unsupported RenderComponentPrimitiveType deserialization");
		}
	}

	void FrameRendererSerializer::DeserializeRenderComponent(IRenderComponent* component, const YAML::Node& componentNode)
	{
		PE_PROFILE_FUNCTION();
		const RenderComponentType type = StringToRenderComponentType(componentNode["Type"].as<std::string>());
		switch (type)
		{
			case RenderComponentType::None:
				PE_CORE_WARN("Attempting to deserialize RenderComponentType::None");
				break;
			case RenderComponentType::Framebuffer:
				DeserializeComponent(static_cast<RenderComponentFramebuffer*>(component), componentNode);
				break;
			case RenderComponentType::Texture:
				DeserializeComponent(static_cast<RenderComponentTexture*>(component), componentNode);
				break;
			case RenderComponentType::Camera:
				DeserializeComponent(static_cast<RenderComponentCamera*>(component), componentNode);
				break;
			case RenderComponentType::Material:
				DeserializeComponent(static_cast<RenderComponentMaterial*>(component), componentNode);
				break;
			case RenderComponentType::UBO:
				DeserializeComponent(static_cast<RenderComponentUBO*>(component), componentNode);
				break;
			case RenderComponentType::FramebufferAttachment:
				DeserializeComponent(static_cast<RenderComponentFBOAttachment*>(component), componentNode);
				break;
			case RenderComponentType::PrimitiveType:
				DeserializeComponent(static_cast<IRenderComponentPrimitiveType*>(component), componentNode);
				break;
			case RenderComponentType::EnvironmentMap:
				DeserializeComponent(static_cast<RenderComponentEnvironmentMap*>(component), componentNode);
				break;
		}
	}

	void FrameRendererSerializer::SerializeRenderer(const FrameRenderer& renderer, const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "FrameRendererConfig" << YAML::Value << path.stem().string();
		out << YAML::Key << "SerializedResources" << YAML::Value << YAML::BeginSeq;

		for (std::string resourceName : renderer.m_SerializedComponentNames)
		{
			const IRenderComponent* component = renderer.m_RenderResources.at(resourceName).get();
			if (component)
			{
				SerializeRenderComponent(component, resourceName.c_str(), out);
			}
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);
		std::ofstream fout = std::ofstream(path);
		fout << out.c_str();
	}

	bool FrameRendererSerializer::DeserializeRenderer(FrameRenderer& renderer, const std::filesystem::path& path)
	{
		PE_PROFILE_FUNCTION();
		std::ifstream stream = std::ifstream(path);
		std::stringstream ss;
		ss << stream.rdbuf();

		const YAML::Node& data = YAML::Load(ss.str());
		if (!data["FrameRendererConfig"]) { return false; }

		std::string configName = data["FrameRendererConfig"].as<std::string>();
		PE_CORE_TRACE("Deserializing renderer config '{0}'", configName);

		const YAML::Node& resources = data["SerializedResources"];
		if (resources) {
			// First, create empty entities
			for (const YAML::Node& componentNode : resources)
			{
				std::string name = componentNode["Name"].as<std::string>();

				// Get render component by name
				IRenderComponent* component = renderer.GetRenderResource(name);
				if (component)
				{
					DeserializeRenderComponent(component, componentNode);
				}
			}
		}
		return true;
	}
}