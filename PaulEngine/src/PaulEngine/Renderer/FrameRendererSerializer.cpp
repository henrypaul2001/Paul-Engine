#include "pepch.h"
#include "FrameRendererSerializer.h"

namespace PaulEngine
{
	// Primitive type dispatch table
	const std::unordered_map<std::type_index, DispatchFunc> FrameRendererSerializer::s_PrimitiveTypeDispatch = {
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

		auto it = s_PrimitiveTypeDispatch.find(typeIndex);
		if (it != s_PrimitiveTypeDispatch.end())
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
}